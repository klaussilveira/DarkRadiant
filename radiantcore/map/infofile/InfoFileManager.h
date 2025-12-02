#pragma once

#include <set>
#include "imapinfofile.h"

namespace map
{

class InfoFileManager :
	public IMapInfoFileManager
{
private:
	std::set<IMapInfoFileModulePtr> _modules;

public:
	void registerInfoFileModule(const IMapInfoFileModulePtr& module) override;
	void unregisterInfoFileModule(const IMapInfoFileModulePtr& module) override;

	void foreachModule(const std::function<void(IMapInfoFileModule&)>& functor) override;

	// Module interface
	std::string getName() const override;
	void shutdownModule() override;
};

}

