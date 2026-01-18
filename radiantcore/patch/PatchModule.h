#pragma once

#include <sigc++/connection.h>
#include "ipatch.h"
#include "PatchSettings.h"

namespace patch
{

class PatchModule :
	public IPatchModule
{
private:
	std::unique_ptr<PatchSettings> _settings;

	sigc::connection _patchTextureChanged;

public:
	// PatchCreator implementation
	scene::INodePtr createPatch(PatchDefType type) override;

	IPatchSettings& getSettings() override;

	// RegisterableModule implementation
	std::string getName() const override;
	StringSet getDependencies() const override;
	void initialiseModule(const IApplicationContext& ctx) override;
	void shutdownModule() override;

private:
	void registerPatchCommands();
};

}
