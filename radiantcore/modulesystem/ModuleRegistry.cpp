#include "ModuleRegistry.h"

#include "i18n.h"
#include "iradiant.h"
#include "itextstream.h"
#include <stdexcept>
#include <iostream>
#include "ModuleLoader.h"

#include <fmt/format.h>

namespace module
{

ModuleRegistry::ModuleRegistry(const IApplicationContext& ctx) :
    _context(ctx),
    _loader(new ModuleLoader(*this))
{
    rMessage() << "ModuleRegistry instantiated." << std::endl;

    // Initialise the Reference in the GlobalModuleRegistry() accessor.
    RegistryReference::Instance().setRegistry(*this);
}

ModuleRegistry::~ModuleRegistry()
{
    // The modules map might be non-empty if the app is failing during very
    // early startup stages, and unloadModules() might not have been called yet.
    // Some modules might need to call this instance during their own destruction,
    // so it's better not to rely on the shared_ptr to destruct them.
    unloadModules();
}

void ModuleRegistry::unloadModules()
{
    // Clear out and destroy any modules that were not initialised this session
    _uninitialisedModules.clear();
    _lazyModules.clear();

    // greebo: It's entirely possible that the clear() method will clear the
    // last shared_ptr of a module. Module might still call this class' moduleExists()
    // method which in turn refers to a semi-destructed ModulesMap instance.
    // So, copy the contents to a temporary map before clearing it out.
    ModulesMap tempMap;
    tempMap.swap(_initialisedModules);

    tempMap.clear();

    // Send out the signal that the DLLs/SOs will be unloaded
    signal_modulesUnloading().emit();
    signal_modulesUnloading().clear();

    _loader->unloadModules();
}

void ModuleRegistry::registerModule(const RegisterableModulePtr& module)
{
    assert(module); // don't take NULL module pointers

    if (_modulesInitialised)
    {
        // The train has left, this module is registered too late
        throw std::logic_error(
            "ModuleRegistry: module " + module->getName() +
            " registered after initialisation."
        );
    }

    // Check the compatibility level of this module against our internal one
    if (module->getCompatibilityLevel() != getCompatibilityLevel())
    {
        rError() << "ModuleRegistry: Incompatible module rejected: " << module->getName() <<
            " (module level: " << module->getCompatibilityLevel() << ", registry level: " <<
            getCompatibilityLevel() << ")" << std::endl;
        return;
    }

    // Add this module to the list of uninitialised ones
    const bool wasInserted = _uninitialisedModules.insert({module->getName(), module}).second;

    // Don't allow modules with the same name being added twice
    if (!wasInserted)
    {
        throw std::logic_error(
            "ModuleRegistry: multiple modules named " + module->getName()
        );
    }

    rMessage() << "Module registered: " << module->getName() << std::endl;
}

// Initialise the module (including dependencies, if necessary)
void ModuleRegistry::initialiseModuleRecursive(const std::string& name)
{
    // Check if the module is already initialised
    if (_initialisedModules.find(name) != _initialisedModules.end())
    {
        return;
    }

    // Check if the module exists at all, checking both eager and lazy modules
    ModulesMap::iterator moduleIter = _uninitialisedModules.find(name);
    if (moduleIter == _uninitialisedModules.end())
    {
        moduleIter = _lazyModules.find(name);
        if (moduleIter == _lazyModules.end())
        {
            throw std::logic_error("ModuleRegistry: Module doesn't exist: " + name);
        }
    }

    // Tag this module as "ready" by moving it into the initialised list.
    RegisterableModulePtr module = _initialisedModules.emplace(name, moduleIter->second).first->second;
    const StringSet& dependencies = module->getDependencies();

    // Debug builds should ensure that the dependencies don't reference the
    // module itself directly
    assert(dependencies.find(module->getName()) == dependencies.end());

    // Initialise the dependencies first
    for (const std::string& namedDependency : dependencies)
    {
        try {
            initialiseModuleRecursive(namedDependency);
        }
        catch (const std::logic_error& e) {
            // Rethrow with more information (both the dependency and the dependent module)
            throw std::logic_error(
                "ModuleRegistry: failed to initialise dependency '" + namedDependency
                + "' of module '" + name + "' [" + e.what() + "]"
            );
        }
    }

    const std::size_t moduleCount = _uninitialisedModules.size() + _lazyModules.size();
    _progress = 0.1f + (static_cast<float>(_initialisedModules.size()) / moduleCount) * 0.9f;

    _sigModuleInitialisationProgress.emit(
        fmt::format(_("Initialising Module: {0}"), module->getName()),
        _progress);

    // Initialise the module itself, now that the dependencies are ready
    module->initialiseModule(_context);
}

void ModuleRegistry::initialiseCoreModule()
{
    std::string coreModuleName = MODULE_RADIANT_CORE;

    auto moduleIter = _uninitialisedModules.find(coreModuleName);

    assert(moduleIter != _uninitialisedModules.end());
    assert(_initialisedModules.find(coreModuleName) == _initialisedModules.end());

    // Tag this module as "ready" by inserting it into the initialised list.
    moduleIter = _initialisedModules.emplace(moduleIter->second->getName(), moduleIter->second).first;

    // We assume that the core module doesn't have any dependencies
    assert(moduleIter->second->getDependencies().empty());

    moduleIter->second->initialiseModule(_context);

    _uninitialisedModules.erase(coreModuleName);
}

void ModuleRegistry::loadAndInitialiseModules()
{
    if (_modulesInitialised)
    {
        throw std::runtime_error("ModuleRegistry::initialiseModule called twice.");
    }

    _sigModuleInitialisationProgress.emit(_("Searching for Modules"), 0.0f);

    rMessage() << "ModuleRegistry Compatibility Level is " << getCompatibilityLevel() << std::endl;

    // Invoke the ModuleLoad routine to load the DLLs from modules/ and plugins/
    auto libraryPaths = _context.getLibraryPaths();
    for (auto path : libraryPaths)
    {
        _loader->loadModulesFromPath(path);
    }

    _progress = 0.1f;
    _sigModuleInitialisationProgress.emit(_("Initialising Modules"), _progress);

    // Handle both eager and lazy modules
    for (const auto& [name, module]: _uninitialisedModules)
    {
        if (module->isLazy())
        {
            _lazyModules.insert({name, module});
        }
        else
        {
            // greebo: Dive into the recursion
            // (this will return immediately if the module is already initialised).
            initialiseModuleRecursive(name);
        }
    }

    _uninitialisedModules.clear();

    // Make sure this isn't called again
    _modulesInitialised = true;

    _progress = 1.0f;
    _sigModuleInitialisationProgress.emit(_("Modules initialised"), _progress);

    // Fire the signal now, this will destroy the Splash dialog as well
    // This event only happens once, release the listeners afterwards
    _sigAllModulesInitialised.emit();
    _sigAllModulesInitialised.clear();
}

void ModuleRegistry::shutdownModules()
{
    if (_modulesShutdown)
    {
        throw std::logic_error("ModuleRegistry: shutdownModules called twice.");
    }

    _sigModulesUninitialising.emit();
    _sigModulesUninitialising.clear();

    for (ModulesMap::value_type& pair : _initialisedModules)
    {
        pair.second->shutdownModule();
    }

    // Fire the signal before unloading the modules, clear the listeners afterwards
    _sigAllModulesUninitialised.emit();
    _sigAllModulesUninitialised.clear();

    // Free all the shared ptrs
    unloadModules();

    _modulesShutdown = true;
}

bool ModuleRegistry::moduleExists(const std::string& name) const
{
    // Try to find the initialised module, uninitialised don't count as existing
    return _initialisedModules.find(name) != _initialisedModules.end();
}

// Get the module
RegisterableModulePtr ModuleRegistry::getModule(const std::string& name)
{
    // Try to find an already initialised module
    if (const auto i = _initialisedModules.find(name); i != _initialisedModules.end())
    {
        return i->second;
    }

    // Nothing initialised with this name; see if it is a lazy module
    if (auto lazyIter = _lazyModules.find(name); lazyIter != _lazyModules.end())
    {
        rMessage() << "ModuleRegistry: initialising lazy module '" << name << "'\n";
        initialiseModuleRecursive(name);

        // Module should be initialised now
        if (auto initIter = _initialisedModules.find(name);
            initIter != _initialisedModules.end())
        {
            assert(lazyIter->second.get() == initIter->second.get());

            // Remove from list of lazy modules before returning the pointer
            _lazyModules.erase(lazyIter);
            return initIter->second;
        }
        else
        {
            throw std::logic_error(
                "ModuleRegistry: lazy module '" + name + "' failed to initialise"
            );
        }
    }

    // Not found at all
    rConsoleError() << "ModuleRegistry: Warning! Module with name " << name
                    << " requested but not found!" << std::endl;
    return {};
}

const IApplicationContext& ModuleRegistry::getApplicationContext() const
{
    return _context;
}

applog::ILogWriter& ModuleRegistry::getApplicationLogWriter()
{
    auto moduleIter = _initialisedModules.find(MODULE_RADIANT_CORE);

    if (moduleIter == _initialisedModules.end())
    {
        throw std::runtime_error("Core module not available.");
    }

    auto coreModule = std::dynamic_pointer_cast<radiant::IRadiant>(moduleIter->second);
    assert(coreModule);

    return coreModule->getLogWriter();
}

sigc::signal<void>& ModuleRegistry::signal_allModulesInitialised()
{
    return _sigAllModulesInitialised;
}

ModuleRegistry::ProgressSignal& ModuleRegistry::signal_moduleInitialisationProgress()
{
    return _sigModuleInitialisationProgress;
}

sigc::signal<void>& ModuleRegistry::signal_modulesUninitialising()
{
    return _sigModulesUninitialising;
}

sigc::signal<void>& ModuleRegistry::signal_allModulesUninitialised()
{
    return _sigAllModulesUninitialised;
}

sigc::signal<void>& ModuleRegistry::signal_modulesUnloading()
{
    return _sigModulesUnloading;
}

std::size_t ModuleRegistry::getCompatibilityLevel() const
{
    return MODULE_COMPATIBILITY_LEVEL;
}

std::string ModuleRegistry::getModuleList(const std::string& separator)
{
    std::string returnValue;

    for (ModulesMap::value_type& pair : _initialisedModules)
    {
        returnValue += (returnValue.empty()) ? "" : separator;
        returnValue += pair.first;
    }

    return returnValue;
}

} // namespace module
