#ifndef _RENDERSYSTEM_FACTORY_IMPL_H_
#define _RENDERSYSTEM_FACTORY_IMPL_H_

#include "irendersystemfactory.h"

namespace render
{

class RenderSystemFactory :
	public IRenderSystemFactory
{
public:
	RenderSystemPtr createRenderSystem();

	// RegisterableModule implementation
	std::string getName() const;
};

} // namespace

#endif /* _RENDERSYSTEM_FACTORY_IMPL_H_ */
