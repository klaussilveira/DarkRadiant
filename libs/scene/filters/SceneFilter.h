#pragma once

#include <string>
#include "ifilter.h"

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

	/** Construct an SceneFilter with the given name.
	 * Pass the read-only flag to indicate whether this filter is
	 * custom or coming from the "stock" filters in the .game files.
	 */
	SceneFilter(const std::string& name, bool readOnly);

	// Noncopyable
	SceneFilter(const SceneFilter&) = delete;
	SceneFilter& operator=(const SceneFilter&) = delete;

	/** Add a (non entitykeyvalue) rule to this filter.
	 *
	 * @param type
	 * The type of rule - "texture", "entityclass" or "object".
	 *
	 * @param match
	 * The regex match expression to use for this rule.
	 *
	 * @param show
	 * true if this filter should show its matches, false if it should
	 * hide them.
	 */
	void addRule(const FilterRule::Type type, const std::string& match, bool show)
	{
		_rules.push_back(FilterRule::Create(type, match, show));
	}

	/** Add an entitykeyvalue rule to this filter.
	 *
	 * @param key
	 * The key to match.
	 *
	 * @param match
	 * The regex expression to match against the spawnarg value.
	 *
	 * @param show
	 * true if this filter should show its matches, false if it should
	 * hide them.
	 */
	void addEntityKeyValueRule(const std::string& key, const std::string& match, bool show)
	{
		_rules.push_back(FilterRule::CreateEntityKeyValueRule(key, match, show));
	}

	/** Test a given item for visibility against all of the rules
	 * in this SceneFilter.
	 *
	 * @param type
	 * Class of the item to test - "texture", "entityclass" etc
	 *
	 * @param name
	 * String name of the item to test.
	 */
	bool isVisible(const FilterRule::Type type, const std::string& name) const;

	/** Test a given entity for visibility against all of the rules
	 * in this SceneFilter.
	 *
	 * @param type
	 * Class of the item to test - "texture", "entityclass" etc
	 *
	 * @param entity
	 * The entity to test
	 */
	bool isEntityVisible(const FilterRule::Type type, const Entity& entity) const;

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
