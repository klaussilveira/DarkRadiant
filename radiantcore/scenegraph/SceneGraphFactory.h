#pragma once

#include "iscenegraphfactory.h"

namespace scene
{

class SceneGraphFactory :
	public ISceneGraphFactory
{
public:
	GraphPtr createSceneGraph();

	// RegisterableModule implementation
	std::string getName() const;
};
typedef std::shared_ptr<SceneGraphFactory> SceneGraphFactoryPtr;

} // namespace
