#pragma once

#include "i18n.h"
#include "ui/iusercontrol.h"
#include "DecalShooterPanel.h"

namespace ui
{

class DecalShooterControl :
    public IUserControlCreator
{
public:
    std::string getControlName() override
    {
        return UserControl::DecalShooter;
    }

    std::string getDisplayName() override
    {
        return _("Decal Shooter");
    }

    std::string getIcon() override
    {
        return "create_decals.png";
    }

    wxWindow* createWidget(wxWindow* parent) override
    {
        return new DecalShooterPanel(parent);
    }
};

}
