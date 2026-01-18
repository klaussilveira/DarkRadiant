#pragma once

#include "imapformat.h"

namespace map
{

namespace
{
    const float MAP_VERSION_D3 = 2;
}

/// MapFormat implementation for Doom 3
class Doom3MapFormat :
    public MapFormat,
    public std::enable_shared_from_this<Doom3MapFormat>
{
public:
    // RegisterableModule implementation
    std::string getName() const override;
    StringSet getDependencies() const override;
    void initialiseModule(const IApplicationContext& ctx) override;
    void shutdownModule() override;

    // MapFormat implementation
    const std::string& getMapFormatName() const override;
    const std::string& getGameType() const override;
    IMapReaderPtr getMapReader(IMapImportFilter& filter) const override;
    IMapWriterPtr getMapWriter() const override;
    bool canLoad(std::istream& stream) const override;

    // Utility function to check a D3-style version number in the map stream
    static bool hasMapVersion(std::istream& stream, float mapVersion);
};
typedef std::shared_ptr<Doom3MapFormat> Doom3MapFormatPtr;

} // namespace map
