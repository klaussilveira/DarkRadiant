#include "SceneFilter.h"

#include "scene/Entity.h"
#include "scene/EntityClass.h"
#include "ifilter.h"
#include <regex>
#include <algorithm>

namespace filters
{

SceneFilter::SceneFilter(const std::string& name, bool readOnly):
    _name(name),
    _readonly(readOnly)
{
    updateEventName();
}

SceneFilter::SceneFilter(const xml::Node& node, bool readOnly):
    _readonly(readOnly),
    _name(node.getAttributeValue("name"))
{
    // Get all of the filterCriterion children of this node
    xml::NodeList critNodes = node.getNamedChildren("filterCriterion");

    // Create XMLFilterRule objects for each criterion
    for (const auto& critNode : critNodes)
    {
        std::string typeStr = critNode.getAttributeValue("type");
        bool show = critNode.getAttributeValue("action") == "show";
        std::string match = critNode.getAttributeValue("match");

        if (typeStr == "texture")
        {
            addRule(filters::TextureQuery{match}, show);
        }
        else if (typeStr == "entityclass")
        {
            addRule(filters::EntityClassQuery{match}, show);
        }
        else if (typeStr == "object")
        {
            addRule(
                filters::PrimitiveQuery{
                    match == "brush" ? PrimitiveType::Brush : PrimitiveType::Patch
                },
                show
            );
        }
        else if (typeStr == "entitykeyvalue")
        {
            addRule(
                filters::SpawnArgQuery{critNode.getAttributeValue("key"), match}, show
            );
        }
    }

    updateEventName();
}

void SceneFilter::saveToNode(xml::Node& parentNode)
{
    // Create the <filter> node for this filter
    xml::Node filterNode = parentNode.createChild("filter");
    filterNode.setAttributeValue("name", getName());

    // Save all the rules as children to that node
    for (const FilterRule& rule: _rules)
    {
        // Create a new criterion tag
        xml::Node criterion = filterNode.createChild("filterCriterion");

        if (rule.type == FilterType::SPAWNARG) {
            criterion.setAttributeValue("key", rule.entityKey);
        }
        criterion.setAttributeValue("type", rule.getTypeString());
        criterion.setAttributeValue("match", rule.match);
        criterion.setAttributeValue("action", rule.show ? "show" : "hide");
    }
}

bool SceneFilter::isVisible(const FilterType type, const std::string& name) const
{
    // Iterate over the rules in this filter, checking if each one is a rule for
    // the chosen item. If so, test the match expression and retrieve the visibility
    // flag if there is a match.

    bool visible = true; // default if unmodified by rules

    for (const FilterRule& rule: _rules)
    {
        // Check the item type.
        if (rule.type != type)
            continue;

        // If we have a rule for this item, use a regex to match the query name
        // against the "match" parameter
        if (const std::regex ex(rule.match); std::regex_match(name, ex))
        {
            // Overwrite the visible flag with the value from the rule.
            visible = rule.show;
        }
    }

    // Pass back the current visibility value
    return visible;
}

bool SceneFilter::isEntityVisible(const Entity& entity) const
{
    bool visible = true; // default if unmodified by rules

    for (const FilterRule& rule: _rules)
    {
        if (rule.type == FilterType::ECLASS)
        {
            scene::EntityClass::CPtr eclass = entity.getEntityClass();
            if (const std::regex ex(rule.match); std::regex_match(eclass->getDeclName(), ex))
            {
                visible = rule.show;
            }
        }
        else if (rule.type == FilterType::SPAWNARG)
        {
            if (const std::regex ex(rule.match);
                std::regex_match(entity.getKeyValue(rule.entityKey), ex))
            {
                visible = rule.show;
            }
        }
    }

    return visible;
}

void SceneFilter::setName(const std::string& newName) {
    // Set the name ...
    _name = newName;

    // ...and update the event name
    updateEventName();
}

void SceneFilter::setRules(const FilterRules& rules) {
    _rules = rules;
}

void SceneFilter::updateEventName() {
    // Construct the eventname out of the filtername (strip the spaces and add "Filter" prefix)
    _eventName = _name;

    // Strip all spaces from the string
    _eventName.erase(std::remove(_eventName.begin(), _eventName.end(), ' '), _eventName.end());

    _eventName = "Filter" + _eventName;
}

} // namespace filters
