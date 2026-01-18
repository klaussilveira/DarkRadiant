#include "FilterPopupMenu.h"

#include "ifilter.h"
#include <wx/menu.h>

namespace wxutil
{

FilterPopupMenu::FilterPopupMenu()
{
    // Visit the filters in the FilterSystem to populate the menu
    GlobalFilterSystem().forEachFilter([=](const filters::SceneFilter& f) {
        // Allocate an ID for this filter
        const int menuItemID = _filterItems.size();
        _filterItems.push_back(f.getName());

        // Add a checkable menu item, initialised with the current filter state
        auto* item = AppendCheckItem(menuItemID, f.getName());
        item->Check(GlobalFilterSystem().getFilterState(f.getName()));
        Bind(wxEVT_MENU, &FilterPopupMenu::menuItemToggled, this, menuItemID);
    });
}

void FilterPopupMenu::menuItemToggled(wxCommandEvent& ev)
{
    GlobalFilterSystem().setFilterState(_filterItems.at(ev.GetId()), ev.IsChecked());
}

} // namespace
