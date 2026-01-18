#include "NamespaceFactory.h"

#include "Namespace.h"
#include "module/StaticModule.h"

INamespacePtr NamespaceFactory::createNamespace()
{
	return std::make_shared<Namespace>();
}

// RegisterableModule implementation
std::string NamespaceFactory::getName() const
{
	static std::string _name(MODULE_NAMESPACE_FACTORY);
	return _name;
}

// Define the static NamespaceFactoryModule
module::StaticModuleRegistration<NamespaceFactory> namespaceFactoryModule;
