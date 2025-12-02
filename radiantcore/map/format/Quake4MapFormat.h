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
	std::string getName() const;
	StringSet getDependencies() const;
	void initialiseModule(const IApplicationContext& ctx);
	void shutdownModule();

	virtual const std::string& getMapFormatName() const;
	virtual const std::string& getGameType() const;
	virtual IMapReaderPtr getMapReader(IMapImportFilter& filter) const;
	virtual IMapWriterPtr getMapWriter() const;

	virtual bool allowInfoFileCreation() const;

	virtual bool canLoad(std::istream& stream) const;
};
typedef std::shared_ptr<Quake4MapFormat> Quake4MapFormatPtr;

} // namespace
