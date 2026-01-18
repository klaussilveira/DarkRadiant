#include "FilterGroup.h"

namespace filters
{

FilterGroup::FilterGroup(const xml::Node& node)
{
    if (node.getName() != "filterGroup")
    {
        throw std::logic_error(
            "FilterGroup: expected 'filterGroup' node, got '" + node.getName() + "'"
        );
    }

    // Read the group name
    _name = xml::getKeyValue(node, "name");

    // Look for contained filters in child nodes
    if (xml::Node childRoot = node.getChild("filters"); childRoot.isValid())
    {
        for (const xml::Node& child: childRoot.getNamedChildren("filter"))
        {
            _filterNames.insert(child.getContent());
        }
    }
}

}