#include "RenderSystemFactory.h"

#include "module/StaticModule.h"

#include "OpenGLRenderSystem.h"

namespace render
{

RenderSystemPtr RenderSystemFactory::createRenderSystem()
{
    return std::make_shared<OpenGLRenderSystem>();
}

std::string RenderSystemFactory::getName() const
{
	static std::string _name(MODULE_RENDERSYSTEMFACTORY);
	return _name;
}

// Define the static RenderSystemFactory module
module::StaticModuleRegistration<RenderSystemFactory> renderSystemFactory;

} // namespace
