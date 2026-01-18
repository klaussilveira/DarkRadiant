#pragma once

#include "imapformat.h"

namespace map
{

namespace
{
    const float MAP_VERSION_Q4 = 3;
}

class Quake4MapFormat :
    public MapFormat,
    public std::enable_shared_from_this<Quake4MapFormat>
{
public:
    // RegisterableModule implementation
    std::string getName() const override;
    StringSet getDependencies() const override;
    void initialiseModule(const IApplicationContext& ctx) override;
    void shutdownModule() override;

    virtual const std::string& getMapFormatName() const override;
    virtual const std::string& getGameType() const override;
    virtual IMapReaderPtr getMapReader(IMapImportFilter& filter) const override;
    virtual IMapWriterPtr getMapWriter() const override;

    virtual bool canLoad(std::istream& stream) const override;
};
typedef std::shared_ptr<Quake4MapFormat> Quake4MapFormatPtr;

} // namespace
