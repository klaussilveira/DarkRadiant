#pragma once

#include "Doom3MapFormat.h"

namespace map
{

/**
 * The prefab format is a specialised Doom3 Map Format, mainly to
 * ensure that no layer information is saved and loaded.
 */
class Doom3PrefabFormat :
	public Doom3MapFormat
{
public:
	// Override some RegisterableModule implementation
	std::string getName() const;
	void initialiseModule(const IApplicationContext& ctx);
	void shutdownModule();

	virtual const std::string& getMapFormatName() const;
	virtual bool allowInfoFileCreation() const;
};
typedef std::shared_ptr<Doom3PrefabFormat> Doom3PrefabFormatPtr;

} // namespace
