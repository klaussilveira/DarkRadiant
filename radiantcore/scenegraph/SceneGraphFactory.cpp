#include "SceneGraphFactory.h"

#include "SceneGraph.h"

namespace scene
{

GraphPtr SceneGraphFactory::createSceneGraph()
{
	return std::make_shared<SceneGraph>();
}

std::string SceneGraphFactory::getName() const
{
	static std::string _name(MODULE_SCENEGRAPHFACTORY);
	return _name;
}

} // namespace
