#pragma once

#include "wxutil/menu/PopupMenu.h"

namespace wxutil
{

/** 
 * Utility class for generating a Filters fly-out menu. 
 * Provides a menu with a check button for each of the registered filters.
 */
class FilterPopupMenu: public PopupMenu
{
    std::vector<std::string> _filterItems;

public:
    // Constructs the filter items
    FilterPopupMenu();

private:
    void menuItemToggled(wxCommandEvent&);
};

} // namespace
