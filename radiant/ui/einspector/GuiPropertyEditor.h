#pragma once

#include "PropertyEditor.h"

namespace ui
{

class GuiPropertyEditor :
	public PropertyEditor
{
private:
	ITargetKey::Ptr _key;

protected:
	void onBrowseButtonClick() override;

public:
	GuiPropertyEditor(wxWindow* parent, IEntitySelection& entities, const ITargetKey::Ptr& key);

	static Ptr CreateNew(wxWindow* parent, IEntitySelection& entities, const ITargetKey::Ptr& key)
	{
		return std::make_shared<GuiPropertyEditor>(parent, entities, key);
	}
};

} // namespace
