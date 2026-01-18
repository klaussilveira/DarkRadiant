#pragma once

#include "inamespace.h"

class NamespaceFactory :
	public INamespaceFactory
{
public:
	/**
	 * Creates and returns a new Namespace.
	 */
	virtual INamespacePtr createNamespace() override;

	// RegisterableModule implementation
	std::string getName() const override;
};

