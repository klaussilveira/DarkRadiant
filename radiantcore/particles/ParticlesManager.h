#pragma once

#include "ParticleDef.h"

#include "iparticles.h"

#include <sigc++/connection.h>

namespace particles
{

class ParticlesManager :
	public IParticlesManager
{
private:
    // Reloaded signal
    sigc::connection _defsReloadedConn;
    sigc::signal<void> _particlesReloadedSignal;

public:
	// IParticlesManager implementation
    sigc::signal<void>& signal_particlesReloaded() override;

    void forEachParticleDef(const ParticleDefVisitor& visitor) override;

    IParticleDef::Ptr getDefByName(const std::string& name) override;

	IParticleDef::Ptr findOrInsertParticleDef(const std::string& name) override;

	void removeParticleDef(const std::string& name) override;

    IRenderableParticlePtr getRenderableParticle(const std::string& name) override;
    IParticleNodePtr createParticleNode(const std::string& name) override;

	void saveParticleDef(const std::string& particle) override;

	// RegisterableModule implementation
	std::string getName() const override;
    StringSet getDependencies() const override;
    void initialiseModule(const IApplicationContext& ctx) override;
    void shutdownModule() override;

private:
    ParticleDefPtr findOrInsertParticleDefInternal(const std::string& name);
};

}
