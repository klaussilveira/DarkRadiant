#pragma once

#include "igl.h"

namespace gl
{

class SharedOpenGLContextModule :
	public ISharedGLContextHolder
{
private:
	IGLContext::Ptr _sharedContext;

	sigc::signal<void> _sigSharedContextCreated;
	sigc::signal<void> _sigSharedContextDestroyed;

public:
    const IGLContext::Ptr& getSharedContext() override;
    void setSharedContext(const IGLContext::Ptr& context) override;

    sigc::signal<void>& signal_sharedContextCreated() override;
    sigc::signal<void>& signal_sharedContextDestroyed() override;

	// RegisterableModule implementation
	std::string getName() const override;
	void shutdownModule() override;
};

}
