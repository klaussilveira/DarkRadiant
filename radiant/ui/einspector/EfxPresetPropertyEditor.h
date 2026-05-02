#pragma once

#include "PropertyEditor.h"

namespace ui
{

class EfxPresetPropertyEditor :
    public PropertyEditor
{
private:
    ITargetKey::Ptr _key;

    void onBrowseButtonClick() override;

public:
    EfxPresetPropertyEditor(wxWindow* parent, IEntitySelection& entities, const ITargetKey::Ptr& key);

    static Ptr CreateNew(wxWindow* parent, IEntitySelection& entities, const ITargetKey::Ptr& key)
    {
        return std::make_shared<EfxPresetPropertyEditor>(parent, entities, key);
    }
};

}
