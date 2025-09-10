#pragma once

#include "xmlutil/Node.h"

#include <set>
#include <string>

namespace filters
{

/// A set of SceneFilters which can be applied together in a single operation
class FilterGroup
{
public:
    using FilterNames = std::set<std::string>;

    /// Construct by parsing an XML node
    FilterGroup(const xml::Node& node);

    /// Get the name of this filter group
    const std::string& getName() const { return _name; }

    /// Get the set of contained filter names
    const FilterNames& getFilterNames() const { return _filterNames; }

private:
    // Name of the group itself
    std::string _name;

    // Names of all the constituent filters
    FilterNames _filterNames;

};

}