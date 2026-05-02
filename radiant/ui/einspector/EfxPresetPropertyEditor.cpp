#include "EfxPresetPropertyEditor.h"
#include "PropertyEditorFactory.h"

#include "i18n.h"
#include "wxutil/dialog/DialogBase.h"

#include <wx/listbox.h>
#include <wx/sizer.h>

namespace ui
{

namespace
{

const char* const EFX_PRESETS[] = {
    "GENERIC", "PADDEDCELL", "ROOM", "BATHROOM", "LIVINGROOM",
    "STONEROOM", "AUDITORIUM", "CONCERTHALL", "CAVE", "ARENA",
    "HANGAR", "CARPETEDHALLWAY", "HALLWAY", "STONECORRIDOR", "ALLEY",
    "FOREST", "CITY", "MOUNTAINS", "QUARRY", "PLAIN",
    "PARKINGLOT", "SEWERPIPE", "UNDERWATER", "DRUGGED", "DIZZY",
    "PSYCHOTIC",
    "CASTLE_SMALLROOM", "CASTLE_SHORTPASSAGE", "CASTLE_MEDIUMROOM",
    "CASTLE_LARGEROOM", "CASTLE_LONGPASSAGE", "CASTLE_HALL",
    "CASTLE_CUPBOARD", "CASTLE_COURTYARD", "CASTLE_ALCOVE",
    "FACTORY_SMALLROOM", "FACTORY_SHORTPASSAGE", "FACTORY_MEDIUMROOM",
    "FACTORY_LARGEROOM", "FACTORY_LONGPASSAGE", "FACTORY_HALL",
    "FACTORY_CUPBOARD", "FACTORY_COURTYARD", "FACTORY_ALCOVE",
    "ICEPALACE_SMALLROOM", "ICEPALACE_SHORTPASSAGE", "ICEPALACE_MEDIUMROOM",
    "ICEPALACE_LARGEROOM", "ICEPALACE_LONGPASSAGE", "ICEPALACE_HALL",
    "ICEPALACE_CUPBOARD", "ICEPALACE_COURTYARD", "ICEPALACE_ALCOVE",
    "SPACESTATION_SMALLROOM", "SPACESTATION_SHORTPASSAGE",
    "SPACESTATION_MEDIUMROOM", "SPACESTATION_LARGEROOM",
    "SPACESTATION_LONGPASSAGE", "SPACESTATION_HALL",
    "SPACESTATION_CUPBOARD", "SPACESTATION_ALCOVE",
    "WOODEN_SMALLROOM", "WOODEN_SHORTPASSAGE", "WOODEN_MEDIUMROOM",
    "WOODEN_LARGEROOM", "WOODEN_LONGPASSAGE", "WOODEN_HALL",
    "WOODEN_CUPBOARD", "WOODEN_COURTYARD", "WOODEN_ALCOVE",
    "SPORT_EMPTYSTADIUM", "SPORT_SQUASHCOURT", "SPORT_SMALLSWIMMINGPOOL",
    "SPORT_LARGESWIMMINGPOOL", "SPORT_GYMNASIUM", "SPORT_FULLSTADIUM",
    "SPORT_STADIUMTANNOY",
    "PREFAB_WORKSHOP", "PREFAB_SCHOOLROOM", "PREFAB_PRACTISEROOM",
    "PREFAB_OUTHOUSE", "PREFAB_CARAVAN",
    "DOME_TOMB", "PIPE_SMALL", "DOME_SAINTPAULS", "PIPE_LONGTHIN",
    "PIPE_LARGE", "PIPE_RESONANT",
    "OUTDOORS_BACKYARD", "OUTDOORS_ROLLINGPLAINS", "OUTDOORS_DEEPCANYON",
    "OUTDOORS_CREEK", "OUTDOORS_VALLEY",
    "MOOD_HEAVEN", "MOOD_HELL", "MOOD_MEMORY",
    "DRIVING_COMMENTATOR", "DRIVING_PITGARAGE", "DRIVING_INCAR_RACER",
    "DRIVING_INCAR_SPORTS", "DRIVING_INCAR_LUXURY",
    "DRIVING_FULLGRANDSTAND", "DRIVING_EMPTYGRANDSTAND", "DRIVING_TUNNEL",
    "CITY_STREETS", "CITY_SUBWAY", "CITY_MUSEUM", "CITY_LIBRARY",
    "CITY_UNDERPASS", "CITY_ABANDONED",
    "DUSTYROOM", "CHAPEL", "SMALLWATERROOM",
};

class EfxPresetChooser :
    public wxutil::DialogBase
{
private:
    wxListBox* _listBox;

public:
    EfxPresetChooser(wxWindow* parent, const std::string& preselect) :
        DialogBase(_("Choose EFX Preset"), parent, "EfxPresetChooser")
    {
        auto* sizer = new wxBoxSizer(wxVERTICAL);

        _listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(320, 400),
            0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);

        for (const char* name : EFX_PRESETS)
        {
            _listBox->Append(name);
        }

        if (!preselect.empty())
        {
            int idx = _listBox->FindString(preselect);
            if (idx != wxNOT_FOUND)
            {
                _listBox->SetSelection(idx);
            }
        }

        _listBox->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent&)
        {
            if (_listBox->GetSelection() != wxNOT_FOUND) EndModal(wxID_OK);
        });

        sizer->Add(_listBox, 1, wxEXPAND | wxALL, 12);
        sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 12);

        SetSizerAndFit(sizer);
    }

    std::string getSelection() const
    {
        int idx = _listBox->GetSelection();
        return idx == wxNOT_FOUND ? std::string() : _listBox->GetString(idx).ToStdString();
    }
};

}

EfxPresetPropertyEditor::EfxPresetPropertyEditor(wxWindow* parent, IEntitySelection& entities, const ITargetKey::Ptr& key) :
    PropertyEditor(entities),
    _key(key)
{
    constructBrowseButtonPanel(parent, _("Choose EFX preset..."),
        PropertyEditorFactory::getBitmapFor("sound"));
}

void EfxPresetPropertyEditor::onBrowseButtonClick()
{
    EfxPresetChooser dialog(wxGetTopLevelParent(getWidget()),
        getKeyValueFromSelection(_key->getFullKey()));

    if (dialog.ShowModal() == wxID_OK)
    {
        std::string picked = dialog.getSelection();
        if (!picked.empty())
        {
            setKeyValueOnSelection(_key->getFullKey(), picked);
        }
    }
}

}
