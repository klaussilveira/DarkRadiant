#pragma once

#include "imapformat.h"

namespace map
{

namespace format
{

class PortableMapFormat :
    public MapFormat,
    public std::enable_shared_from_this<PortableMapFormat>
{
public:
    // Format version, will be exported as <map> tag attribute
    static std::size_t Version;
    static const char* Name;

    typedef std::shared_ptr<PortableMapFormat> Ptr;

    // RegisterableModule implementation
    std::string getName() const override;
    StringSet getDependencies() const override;
    void initialiseModule(const IApplicationContext& ctx) override;
    void shutdownModule() override;

    const std::string& getMapFormatName() const override;
    const std::string& getGameType() const override;
    IMapReaderPtr getMapReader(IMapImportFilter& filter) const override;
    IMapWriterPtr getMapWriter() const override;

    bool allowInfoFileCreation() const override
    {
        return false;
    }

    bool canLoad(std::istream& stream) const override;
};

}

} // namespace map
