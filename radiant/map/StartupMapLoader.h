#pragma once

#include "imodule.h"

namespace map
{

class StartupMapLoader :
	public RegisterableModule
{
public:
	std::string getName() const override;
	StringSet getDependencies() const override;
	void initialiseModule(const IApplicationContext& ctx) override;

private:
	// This gets called as soon as the mainframe is shown
	void onMainFrameReady();

	void loadMapSafe(const std::string& map);
};

} // namespace map
