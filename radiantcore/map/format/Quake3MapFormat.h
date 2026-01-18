#pragma once

#include "imapformat.h"

namespace map
{

class Quake3MapFormatBase :
	public MapFormat
{
public:
	// RegisterableModule implementation
	StringSet getDependencies() const override;
	void initialiseModule(const IApplicationContext& ctx) override;
	void shutdownModule() override;

    // Map reader is shared by Q3 and Q3 alternate
	virtual IMapReaderPtr getMapReader(IMapImportFilter& filter) const override;

	virtual bool canLoad(std::istream& stream) const override;

protected:
    virtual std::shared_ptr<Quake3MapFormatBase> getSharedToThis() = 0;
};

class Quake3MapFormat :
	public Quake3MapFormatBase,
	public std::enable_shared_from_this<Quake3MapFormat>
{
public:
	// RegisterableModule implementation
	std::string getName() const override;

	virtual const std::string& getMapFormatName() const override;
	virtual const std::string& getGameType() const override;
	virtual IMapWriterPtr getMapWriter() const override;

protected:
    virtual std::shared_ptr<Quake3MapFormatBase> getSharedToThis() override
    {
        return shared_from_this();
    }
};

class Quake3AlternateMapFormat :
    public Quake3MapFormatBase,
    public std::enable_shared_from_this<Quake3AlternateMapFormat>
{
public:
    // RegisterableModule implementation
    std::string getName() const override;

    virtual const std::string& getMapFormatName() const override;
    virtual const std::string& getGameType() const override;
    virtual IMapWriterPtr getMapWriter() const override;

protected:
    virtual std::shared_ptr<Quake3MapFormatBase> getSharedToThis() override
    {
        return shared_from_this();
    }
};

} // namespace
