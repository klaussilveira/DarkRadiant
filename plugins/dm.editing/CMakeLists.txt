add_library(dm_editing MODULE
            AIEditingPanel.cpp
            AIHeadChooserDialog.cpp
            AIHeadPropertyEditor.cpp
            AIVocalSetChooserDialog.cpp
            AIVocalSetPreview.cpp
            AIVocalSetPropertyEditor.cpp
            DarkmodTxt.cpp
            FixupMap.cpp
            FixupMapDialog.cpp
            MissionInfoEditDialog.cpp
            MissionInfoGuiView.cpp
            MissionInfoTextFile.cpp
            MissionReadmeDialog.cpp
            plugin.cpp
            ReadmeTxt.cpp)
target_compile_options(dm_editing PUBLIC ${SIGC_CFLAGS})
target_link_libraries(dm_editing PUBLIC wxutil)
set_target_properties(
    dm_editing PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${DR_STAGING_PLUGINSDIR}"
)
