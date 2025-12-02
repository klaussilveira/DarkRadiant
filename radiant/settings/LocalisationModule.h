#pragma once

#include "imodule.h"

namespace settings
{

// Localisation module, taking care of the Language Preferences page
class LocalisationModule :
	public RegisterableModule
{
public:
	std::string getName() const override;
	StringSet getDependencies() const override;
	void initialiseModule(const IApplicationContext& ctx) override;
	void shutdownModule() override;
};

}
