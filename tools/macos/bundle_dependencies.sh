#!/bin/bash
set -euo pipefail

if [ $# -ne 1 ] || [ ! -d "$1/Contents/MacOS" ]; then
    echo "Usage: $0 <path/to/DarkRadiant.app>" >&2
    exit 1
fi

app="$(cd "$1" && pwd)"
frameworks="$app/Contents/Frameworks"
mkdir -p "$frameworks"

queue=()
origins=()
while IFS= read -r -d '' file; do
    if file "$file" | grep -q "Mach-O"; then
        queue+=("$file")
        origins+=("$file")
    fi
done < <(find "$app/Contents/MacOS" -type f -print0)

is_system_library() {
    [[ "$1" == /System/* || "$1" == /usr/lib/* ]]
}

set_install_name() {
    install_name_tool -id "$2" "$1" 2>/dev/null
}

resolve_relative_dependency() {
    local dependency="$1" origin="$2"
    local origin_dir rpath
    origin_dir="$(dirname "$origin")"

    if [[ "$dependency" == @loader_path/* ]]; then
        echo "$origin_dir/${dependency#@loader_path/}"
        return
    fi

    for rpath in $(otool -l "$origin" | awk '$1 == "cmd" && $2 == "LC_RPATH" { getline; getline; print $2 }'); do
        rpath="${rpath/@loader_path/$origin_dir}"
        if [ -e "$rpath/${dependency#@rpath/}" ]; then
            echo "$rpath/${dependency#@rpath/}"
            return
        fi
    done
}

embed_python_framework() {
    local binary="$1"
    local version_dir version target library
    version_dir="$(dirname "$binary")"
    version="$(basename "$version_dir")"
    target="$frameworks/Python.framework"

    if [ -d "$target" ]; then
        return
    fi

    echo "Embedding Python $version framework"
    mkdir -p "$target/Versions/$version/lib"
    cp "$binary" "$target/Versions/$version/Python"
    cp -R "$version_dir/Resources" "$target/Versions/$version/Resources"
    rm -rf "$target/Versions/$version/Resources/Python.app"
    rsync -a \
        --exclude "test/" --exclude "idlelib/" --exclude "tkinter/" --exclude "turtledemo/" \
        --exclude "ensurepip/" --exclude "config-*/" --exclude "site-packages" --exclude "_tkinter*.so" \
        "$version_dir/lib/python$version" "$target/Versions/$version/lib/"
    ln -s "$version" "$target/Versions/Current"
    ln -s "Versions/Current/Python" "$target/Python"
    ln -s "Versions/Current/Resources" "$target/Resources"

    chmod -R u+w "$target"
    set_install_name "$target/Versions/$version/Python" "@executable_path/../Frameworks/Python.framework/Versions/$version/Python"

    queue+=("$target/Versions/$version/Python")
    origins+=("$binary")
    while IFS= read -r -d '' library; do
        queue+=("$library")
        origins+=("$version_dir/lib/${library#"$target/Versions/$version/lib/"}")
    done < <(find "$target/Versions/$version/lib" -name "*.so" -print0)
}

index=0
while [ $index -lt ${#queue[@]} ]; do
    file="${queue[$index]}"
    origin="${origins[$index]}"
    index=$((index + 1))

    for dependency in $(otool -L "$file" | tail -n +2 | awk '{print $1}'); do
        source="$dependency"

        if [[ "$file" == "$frameworks"/* && ( "$dependency" == @rpath/* || "$dependency" == @loader_path/* ) ]]; then
            source="$(resolve_relative_dependency "$dependency" "$origin")"

            if [ -z "$source" ]; then
                echo "Cannot resolve $dependency of $origin" >&2
                exit 1
            fi
        elif is_system_library "$dependency" || [[ "$dependency" == @* ]]; then
            continue
        fi

        if [[ "$source" == */Python.framework/Versions/*/Python ]]; then
            embed_python_framework "$source"
            version="$(basename "$(dirname "$source")")"
            new_name="@executable_path/../Frameworks/Python.framework/Versions/$version/Python"
        else
            name="$(basename "$(realpath "$source")")"
            new_name="@executable_path/../Frameworks/$name"

            if [ ! -e "$frameworks/$name" ]; then
                echo "Bundling $source"
                cp -L "$source" "$frameworks/$name"
                chmod u+w "$frameworks/$name"
                set_install_name "$frameworks/$name" "$new_name"
                queue+=("$frameworks/$name")
                origins+=("$(realpath "$source")")
            fi
        fi

        install_name_tool -change "$dependency" "$new_name" "$file" 2>/dev/null
    done
done

unresolved=0
for file in "${queue[@]}"; do
    for dependency in $(otool -L "$file" | tail -n +2 | awk '{print $1}'); do
        if [[ "$file" == "$frameworks"/* && ( "$dependency" == @rpath/* || "$dependency" == @loader_path/* ) ]] ||
           { ! is_system_library "$dependency" && [[ "$dependency" != @* ]]; }; then
            echo "Unresolved dependency in $file: $dependency" >&2
            unresolved=1
        fi
    done
done

if [ $unresolved -ne 0 ]; then
    exit 1
fi

for file in "${queue[@]}"; do
    codesign --force --sign - "$file"
done

if [ -d "$frameworks/Python.framework" ]; then
    codesign --force --sign - "$frameworks/Python.framework"
fi

codesign --force --sign - "$app"
codesign --verify --deep --strict "$app"
echo "Bundled ${#queue[@]} binaries into $app"
