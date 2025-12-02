#pragma once

#include "ifx.h"

namespace fx
{

class FxManager final :
    public IFxManager
{
public:
    IFxDeclaration::Ptr findFx(const std::string& name) override;

    // RegisterableModule implementation
    std::string getName() const override;
    StringSet getDependencies() const override;
    void initialiseModule(const IApplicationContext& ctx) override;
};

}
