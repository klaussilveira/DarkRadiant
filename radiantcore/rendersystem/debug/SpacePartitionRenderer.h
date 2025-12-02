#ifndef _SPACE_PARTITION_RENDERER_H_
#define _SPACE_PARTITION_RENDERER_H_

#include "icommandsystem.h"

#include "render/RenderableSpacePartition.h"

namespace render
{

class SpacePartitionRenderer : public RegisterableModule
{
	RenderableSpacePartition _renderableSP;

public:
	std::string getName() const;
	StringSet getDependencies() const;
	void initialiseModule(const IApplicationContext& ctx);
	void shutdownModule();

	void toggle(const cmd::ArgumentList& args);

private:
	void installRenderer();
	void uninstallRenderer();
};

} // namespace render

#endif /* _SPACE_PARTITION_RENDERER_H_ */
