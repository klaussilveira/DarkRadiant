#pragma once

#include "FilterRule.h"
#include "xmlutil/Node.h"

#include <string>
#include <memory>

class Entity;

namespace filters
{

/**
 * @brief Representation of a single scene filter (broadly corresponding to a
 * single entry in the Filters menu).
 *
 * Each filter consists of a name, and a list of filter rules. The class exposes
 * methods to query textures, entityclasses and objects against these rules.
 */
class SceneFilter
{
    // Text name of filter (from game.xml)
    std::string _name;

    // The name of the toggle event
    std::string _eventName;

    // Ordered list of rule objects
    FilterRules _rules;

    // True if this filter can't be changed
    bool _readonly;

public:
    typedef std::shared_ptr<SceneFilter> Ptr;

    /// Construct an SceneFilter with the given name.
    ///
    /// Pass the read-only flag to indicate whether this filter is custom or coming from
    /// the "stock" filters in the .game files.
    SceneFilter(const std::string& name, bool readOnly);

    /// Construct a SceneFilter from an XML node
    SceneFilter(const xml::Node& node, bool readOnly);

    // Noncopyable
    SceneFilter(const SceneFilter&) = delete;
    SceneFilter& operator=(const SceneFilter&) = delete;

    /**
     * @brief Save the contents of this filter to XML.
     *
     * @param parentNode Node under which a <filter> child node should be created, storing
     * the contents of this filter.
     */
    void saveToNode(xml::Node& parentNode);

    /**
     * @brief Add a rule to this filter.
     *
     * @param query
     * The query that the rule should perform
     *
     * @param show
     * true if this filter should show its matches, false if it should hide them. Since
     * all objects are visible by default, the majority of filters will hide their matched
     * items, but "show" rules are useful in multi-rule filters to re-show specific
     * subsets of items that were hidden by an earlier rule.
     */
    void addRule(filters::Query query, bool show = false)
    {
        _rules.push_back({std::move(query), show});
    }

    /**
     * @brief Test a given non-entity item, such as a texture name, against the
     * contained filter rules.
     *
     * @param type
     * Class of the item to test - "texture", "entityclass" etc
     *
     * @param name
     * String name of the item to test.
     */
    bool isVisible(const FilterType type, const std::string& name) const;

    /// Test a given entity for visibility against all of the rules in this SceneFilter.
    bool isEntityVisible(const Entity& entity) const;

    /** greebo: Returns the name of the toggle event associated to this filter.
    * It's lacking any spaces or other incompatible characters, compared to the actual
    * name returned in getName().
     */
    const std::string& getEventName() const
    {
        return _eventName;
    }

    /// The name of this Filter
    const std::string& getName() const
    {
        return _name;
    }

    /**
     * greebo: Renames the filter to <newName>. This also updates the event name.
     */
    void setName(const std::string& newName);

    /// Whether this filter is read-only
    bool isReadOnly() const
    {
        return _readonly;
    }

    /// Returns the ruleset
    const FilterRules& getRuleSet() const
    {
        return _rules;
    }

    // Applies the given ruleset, replacing the existing one.
    void setRules(const FilterRules& rules);

private:
    void updateEventName();
};

}
