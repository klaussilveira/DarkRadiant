#include "Doom3MapFormat.h"
#include "parser/DefTokeniser.h"
#include "Doom3MapReader.h"
#include "Doom3MapWriter.h"

#include "module/StaticModule.h"

namespace map
{

// RegisterableModule implementation
std::string Doom3MapFormat::getName() const {
    static std::string _name("Doom3MapLoader");
    return _name;
}

StringSet Doom3MapFormat::getDependencies() const
{
    static StringSet _dependencies;

    if (_dependencies.empty())
    {
        _dependencies.insert(MODULE_MAPFORMATMANAGER);
    }

    return _dependencies;
}

void Doom3MapFormat::initialiseModule(const IApplicationContext& ctx)
{
    // Register ourselves as map format for maps and regions
    GlobalMapFormatManager().registerMapFormat("map", shared_from_this());
    GlobalMapFormatManager().registerMapFormat("reg", shared_from_this());
}

void Doom3MapFormat::shutdownModule()
{
    // Unregister now that we're shutting down
    GlobalMapFormatManager().unregisterMapFormat(shared_from_this());
}

const std::string& Doom3MapFormat::getMapFormatName() const
{
    static std::string _name = "Doom 3";
    return _name;
}

const std::string& Doom3MapFormat::getGameType() const
{
    static std::string _gameType = "doom3";
    return _gameType;
}

IMapReaderPtr Doom3MapFormat::getMapReader(IMapImportFilter& filter) const
{
    return IMapReaderPtr(new Doom3MapReader(filter));
}

IMapWriterPtr Doom3MapFormat::getMapWriter() const
{
    return IMapWriterPtr(new Doom3MapWriter);
}

bool Doom3MapFormat::canLoad(std::istream& stream) const
{
    return hasMapVersion(stream, MAP_VERSION_D3);
}

bool Doom3MapFormat::hasMapVersion(std::istream& stream, float mapVersion)
{
    // Instantiate a tokeniser to read the first few tokens
    parser::BasicDefTokeniser<std::istream> tok(stream);

    try
    {
        // Require a "Version" token
        if (const std::string token = tok.nextToken(); token != "Version")
            return false;

        // Require specific version, return true on success
        const std::string verTok = tok.nextToken();
        float version;
        return string::tryConvertToFloat(verTok, version) && version == mapVersion;
    }
    catch (parser::ParseException&)
    {}

    return false;
}

module::StaticModuleRegistration<Doom3MapFormat> d3MapModule;

} // namespace map
