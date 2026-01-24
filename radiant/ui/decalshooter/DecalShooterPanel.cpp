#include "DecalShooterPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>

#include "i18n.h"
#include "wxutil/Bitmap.h"
#include "ui/materials/MaterialChooser.h"
#include "ui/materials/MaterialSelector.h"

namespace ui
{

DecalShooterPanel* DecalShooterPanel::_instance = nullptr;

DecalShooterPanel::DecalShooterPanel(wxWindow* parent) :
    DockablePanel(parent),
    _widthCtrl(nullptr),
    _heightCtrl(nullptr),
    _offsetCtrl(nullptr),
    _materialEntry(nullptr),
    _browseButton(nullptr)
{
    _instance = this;
    populateWindow();
}

DecalShooterPanel::~DecalShooterPanel()
{
    if (_instance == this)
    {
        _instance = nullptr;
    }
}

DecalShooterPanel* DecalShooterPanel::getInstance()
{
    return _instance;
}

double DecalShooterPanel::getDecalWidth() const
{
    return _widthCtrl ? _widthCtrl->GetValue() : 128.0;
}

double DecalShooterPanel::getDecalHeight() const
{
    return _heightCtrl ? _heightCtrl->GetValue() : 128.0;
}

double DecalShooterPanel::getDecalOffset() const
{
    return _offsetCtrl ? _offsetCtrl->GetValue() : 0.125;
}

std::string DecalShooterPanel::getDecalMaterial() const
{
    return _materialEntry ? _materialEntry->GetValue().ToStdString() : "textures/common/decal";
}

void DecalShooterPanel::onPanelActivated()
{
}

void DecalShooterPanel::onPanelDeactivated()
{
}

void DecalShooterPanel::populateWindow()
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(4, 2, 6, 12);
    gridSizer->AddGrowableCol(1);

    wxStaticText* widthLabel = new wxStaticText(this, wxID_ANY, _("Width:"));
    _widthCtrl = new wxSpinCtrlDouble(this, wxID_ANY);
    _widthCtrl->SetRange(1.0, 2048.0);
    _widthCtrl->SetValue(128.0);
    _widthCtrl->SetIncrement(8.0);
    _widthCtrl->SetDigits(1);
    gridSizer->Add(widthLabel, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(_widthCtrl, 1, wxEXPAND);

    wxStaticText* heightLabel = new wxStaticText(this, wxID_ANY, _("Height:"));
    _heightCtrl = new wxSpinCtrlDouble(this, wxID_ANY);
    _heightCtrl->SetRange(1.0, 2048.0);
    _heightCtrl->SetValue(128.0);
    _heightCtrl->SetIncrement(8.0);
    _heightCtrl->SetDigits(1);
    gridSizer->Add(heightLabel, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(_heightCtrl, 1, wxEXPAND);

    wxStaticText* offsetLabel = new wxStaticText(this, wxID_ANY, _("Offset:"));
    _offsetCtrl = new wxSpinCtrlDouble(this, wxID_ANY);
    _offsetCtrl->SetRange(0.0, 16.0);
    _offsetCtrl->SetValue(0.125);
    _offsetCtrl->SetIncrement(0.125);
    _offsetCtrl->SetDigits(3);
    _offsetCtrl->SetToolTip(_("Distance from the surface to prevent z-fighting"));
    gridSizer->Add(offsetLabel, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(_offsetCtrl, 1, wxEXPAND);

    wxStaticText* materialLabel = new wxStaticText(this, wxID_ANY, _("Material:"));

    wxBoxSizer* materialSizer = new wxBoxSizer(wxHORIZONTAL);
    _materialEntry = new wxTextCtrl(this, wxID_ANY, "textures/decals/blood1");
    _materialEntry->SetMinSize(wxSize(120, -1));

    _browseButton = new wxBitmapButton(this, wxID_ANY,
        wxutil::GetLocalBitmap("folder16.png"));
    _browseButton->SetToolTip(_("Choose decal material"));
    _browseButton->Bind(wxEVT_BUTTON, &DecalShooterPanel::onBrowseMaterial, this);

    materialSizer->Add(_materialEntry, 1, wxEXPAND | wxRIGHT, 4);
    materialSizer->Add(_browseButton, 0);

    gridSizer->Add(materialLabel, 0, wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(materialSizer, 1, wxEXPAND);

    GetSizer()->Add(gridSizer, 0, wxEXPAND | wxALL, 12);

    wxStaticText* helpText = new wxStaticText(this, wxID_ANY,
        _("Use Ctrl+Shift+Middle-Click\nin the 3D view to place decals."));
    helpText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    GetSizer()->Add(helpText, 0, wxALL, 12);
}

void DecalShooterPanel::onBrowseMaterial(wxCommandEvent& ev)
{
    auto* chooser = new MaterialChooser(this, MaterialSelector::TextureFilter::Regular, _materialEntry);
    chooser->ShowModal();
    chooser->Destroy();
}

} // namespace ui
