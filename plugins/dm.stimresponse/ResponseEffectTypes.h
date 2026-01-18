#pragma once

#include <map>
#include <string>
#include "scene/EntityClass.h"

namespace
{
	const char* const GKEY_RESPONSE_EFFECT_PREFIX = "/stimResponseSystem/responseEffectPrefix";
}

typedef std::map<std::string, scene::EntityClass::Ptr> ResponseEffectTypeMap;

class ResponseEffectTypes
{
	// The actual map where all the name => EClassPtr associations are stored.
	ResponseEffectTypeMap _effectTypes;

	ResponseEffectTypes();

	static std::shared_ptr<ResponseEffectTypes>& InstancePtr();

public:
	// Contains the static instance of this class
	static ResponseEffectTypes& Instance();

	// Frees the static instance of this class
	// Any calls to Instance() after this call will instantiate a new singleton
	static void Clear();

	/** greebo: Returns the entityclass pointer for the given name.
	 */
	scene::EntityClass::Ptr getEClassForName(const std::string& name);

	/** greebo: Returns a reference to the internally stored EffectTypeMap
	 */
	ResponseEffectTypeMap& getMap();

	/** greebo: Retrieves the first available effect name (e.g. "effect_teleport")
	 */
	std::string getFirstEffectName();
};
