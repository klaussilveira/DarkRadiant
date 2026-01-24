#pragma once

#include <sigc++/trackable.h>
#include "wxutil/DockablePanel.h"

class wxSpinCtrlDouble;
class wxTextCtrl;
class wxButton;

namespace ui
{

/**
 * Panel for configuring the Decal Shooter tool settings.
 */
class DecalShooterPanel :
    public wxutil::DockablePanel,
    public sigc::trackable
{
private:
    wxSpinCtrlDouble* _widthCtrl;
    wxSpinCtrlDouble* _heightCtrl;
    wxSpinCtrlDouble* _offsetCtrl;
    wxTextCtrl* _materialEntry;
    wxButton* _browseButton;

    static DecalShooterPanel* _instance;

public:
    DecalShooterPanel(wxWindow* parent);
    ~DecalShooterPanel() override;

    double getDecalWidth() const;
    double getDecalHeight() const;
    double getDecalOffset() const;
    std::string getDecalMaterial() const;

    static DecalShooterPanel* getInstance();

protected:
    void onPanelActivated() override;
    void onPanelDeactivated() override;

private:
    void populateWindow();
    void onBrowseMaterial(wxCommandEvent& ev);
};

} // namespace ui
