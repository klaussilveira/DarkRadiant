#pragma once

#include "imapresource.h"

namespace map
{

class MapResourceManager :
	public IMapResourceManager
{
private:
	ExportEvent _resourceExporting;
	ExportEvent _resourceExported;

public:
	IMapResourcePtr createFromPath(const std::string& path) override;
    IMapResourcePtr createFromArchiveFile(const std::string& archivePath,
        const std::string& filePathWithinArchive) override;

	ExportEvent& signal_onResourceExporting() override;
	ExportEvent& signal_onResourceExported() override;

	// RegisterableModule implementation
	std::string getName() const override;
	StringSet getDependencies() const override;
};

}
