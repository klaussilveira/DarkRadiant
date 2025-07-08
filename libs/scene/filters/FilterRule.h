#pragma once

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <cassert>

/// Available classes of query that a rule can perform
enum class FilterType
{
    /// Match the string name of a material shader
    TEXTURE,

    /// Match the class of an entity (e.g. "func_static")
    ECLASS,

    /// Match particular classes of primitive, e.g. whether something is a brush or patch
    OBJECT,

    /// Match on the value of a particular entity spawnarg
    SPAWNARG,
};

namespace filters
{

/// Query for a particular material shader
struct TextureQuery { std::string match; };

/// Query for a particular entity class
struct EntityClassQuery { std::string match; };

/// Available primitive types to query
enum class PrimitiveType
{
    Brush,
    Patch
};

/// Query for a particular primitive type
struct PrimitiveQuery { PrimitiveType type; };

/// Query for the value of a particular spawnarg
struct SpawnArgQuery
{
    std::string key;
    std::string valueMatch;
};

/// Variant specifying the query of a particular rule
using Query = std::variant<TextureQuery, EntityClassQuery, PrimitiveQuery, SpawnArgQuery>;

}

/// A single rule for hiding or showing objects, maintained by the filter system.
class FilterRule
{
public:
    // The rule type
    FilterType type;

    // The entity key, only applies for type "entitykeyvalue"
    std::string entityKey;

    // the match expression regex
    std::string match;

    // true for action="show", false for action="hide"
    bool show;

private:
    // Private Constructor, use the named constructors below
    FilterRule(const FilterType type_, const std::string& match_, bool show_) :
        type(type_),
        match(match_),
        show(show_)
    {}

    // Alternative private constructor for the entityKeyValue type
    FilterRule(const FilterType type_, const std::string& entityKey_, const std::string& match_, bool show_) :
        type(type_),
        entityKey(entityKey_),
        match(match_),
        show(show_)
    {}

public:
    // Named constructors

    // Regular constructor for the non-entitykeyvalue types
    static FilterRule Create(const FilterType type, const std::string& match, bool show)
    {
        assert(type != FilterType::SPAWNARG);

        return FilterRule(type, match, show);
    }

    // Constructor for the entity key value type
    static FilterRule CreateEntityKeyValueRule(const std::string& key, const std::string& match, bool show)
    {
        return FilterRule(FilterType::SPAWNARG, key, match, show);
    }

    /// Get a string representing the rule type (e.g. for display in the UI)
    std::string getTypeString() const
    {
        switch (type)
        {
            case FilterType::TEXTURE: return "texture";
            case FilterType::OBJECT: return "object";
            case FilterType::ECLASS: return "entityclass";
            case FilterType::SPAWNARG: return "entitykeyvalue";
        };
        throw std::logic_error("Invalid filter type");
    }
};
typedef std::vector<FilterRule> FilterRules;

// Equality comparison for FilterRule
inline bool operator== (const FilterRule& lhs, const FilterRule& rhs)
{
    return lhs.type == rhs.type && lhs.match == rhs.match && lhs.show == rhs.show
        && (lhs.type != FilterType::SPAWNARG || lhs.entityKey == rhs.entityKey);
}

inline bool operator!= (const FilterRule& lhs, const FilterRule& rhs)
{
    return !(lhs == rhs);
}