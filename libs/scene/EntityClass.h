#pragma once

#include "ifilesystem.h"

#include "math/AABB.h"
#include "math/Vector4.h"
#include "string/string.h"
#include "generic/Lazy.h"

#include "parser/DefTokeniser.h"
#include "decl/DeclarationBase.h"

#include <map>
#include <memory>

#include <sigc++/connection.h>

/* FORWARD DECLS */

class Shader;

/**
 * Data structure representing a single attribute on an entity class.
 *
 * \ingroup eclass
 */
class EntityClassAttribute
{
private:

    // Attribute type
    std::string _type;

    // Attribute name
    std::string _name;

    // Value
    std::string _value;

    //  User-friendly description
    std::string _desc;

public:
    /**
     * The key type (string, bool etc.).
     */
    const std::string& getType() const
    {
        return _type;
    }

    void setType(const std::string& type)
    {
        _type = type;
    }

    /// The attribute key name, e.g. "model", "editor_displayFolder" etc
    const std::string& getName() const
    {
        return _name;
    }

    /// Get attribute value
    const std::string& getValue() const
    {
        return _value;
    }

    /// Set attribute value
    void setValue(const std::string& value)
    {
        _value = value;
    }

    /// The help text associated with the key (in the DEF file).
    const std::string& getDescription() const
    {
        return _desc;
    }

    void setDescription(const std::string& desc)
    {
        _desc = desc;
    }

    /// Construct an EntityClassAttribute
    EntityClassAttribute(const std::string& type_,
                         const std::string& name_,
                         const std::string& value_,
                         const std::string& description_ = "")
    : _type(type_),
      _name(name_),
      _value(value_),
      _desc(description_)
    {}
};


namespace scene
{

/**
 * \brief Entity class implementation.
 *
 * An entity class represents a single type of entity that can be created by
 * the EntityCreator. Entity classes are parsed from .DEF files during startup.
 *
 * Entity class attribute names are compared case-insensitively, as in the
 * Entity class.
 *
 * \ingroup eclass
 */
class EntityClass: public decl::DeclarationBase<decl::IDeclaration>
{
public:
    /// EntityClass pointer type
    using Ptr = std::shared_ptr<EntityClass>;
    using CPtr = std::shared_ptr<const EntityClass>;

    // Enumeration of types DarkRadiant is capable of distinguishing when creating entities
    enum class Type
    {
        Generic,            // fixed-size, coloured boxes with and without arrow
        StaticGeometry,     // func_* entities supporting primitives (like worldspawn)
        EntityClassModel,   // non-fixed size entities with a non-empty "model" key set
        Light,              // all classes with editor_light/idLight or inheriting from them
        Speaker,            // special class used for "speaker" entityDefs
    };

private:
    // Parent class pointer (or NULL)
    scene::EntityClass* _parent = nullptr;

    // UI visibility of this entity class
    Lazy<vfs::Visibility> _visibility;

    // Should this entity type be treated as a light?
    bool _isLight = false;

    // Colour of this entity
    Vector4 _colour;

    bool _colourTransparent = false;

    // Does this entity have a fixed size?
    bool _fixedSize;

    // Map of named EntityAttribute structures. EntityAttributes are picked
    // up from the DEF file during parsing. Ignores key case.
    using EntityAttributeMap = std::map<std::string, EntityClassAttribute, string::ILess>;
    EntityAttributeMap _attributes;

    // Flag to indicate inheritance resolved. An EntityClass resolves its
    // inheritance by copying all values from the parent onto the child,
    // after recursively instructing the parent to resolve its own inheritance.
    bool _inheritanceResolved = false;

    // Emitted when contents are reloaded
    sigc::signal<void> _changedSignal;
    bool _blockChangeSignal = false;
    sigc::connection _parentChangedConnection;

private:
    // Resolve inheritance for this class.
    void resolveInheritance();
    vfs::Visibility determineVisibilityFromValues();

    // Clear all contents (done before parsing from tokens)
    void clear();
    void parseEditorSpawnarg(const std::string& key, const std::string& value);
    void setIsLight(bool val);

    // Visit attributes recursively, parent first then child
    using InternalAttrVisitor = std::function<void(const EntityClassAttribute&)>;
    void forEachAttributeInternal(InternalAttrVisitor visitor,
                                  bool editorKeys) const;

    // Return attribute if found, possibly checking parents
    EntityClassAttribute* getAttribute(const std::string&, bool includeInherited = true);

public:

    /// Construct a named EntityClass
    EntityClass(const std::string& name);

    ~EntityClass();

    /// Create a heap-allocated default/empty EntityClass
    static Ptr CreateDefault(const std::string& name);

    // Returns the type of this entity class (as determined after parsing)
    Type getClassType();

    /// Signal emitted when entity class contents are changed or reloaded
    sigc::signal<void>& changedSignal();

    /// Get the parent entity class or NULL if there is no parent
    EntityClass* getParent();

    /// Get the UI visibility of this entity class
    vfs::Visibility getVisibility() override;

    /// Query whether this entity class represents a light.
    bool isLight();

    /* ENTITY CLASS SIZE */

    /// Query whether this entity has a fixed size.
    bool isFixedSize();

    /**
     * Return an AABB representing the declared size of this entity. This is
     * only valid for fixed size entities.
     */
    AABB getBounds();

    /* ENTITY CLASS COLOURS */

    /// Return the display colour of this entity class
    const Vector4& getColour();

    // Overrides the colour defined in the .def files
    void setColour(const Vector4& colour);

    /* ENTITY CLASS ATTRIBUTES */

    /**
     * @brief Get the value of a specified attribute.
     *
     * @return std::string containing the attribute value, or an empty string if the attribute was
     * not found.
     */
    std::string getAttributeValue(const std::string& name,
                                bool includeInherited = true);

    // Returns the attribute type string for the given name.
    // This method will walk up the inheritance hierarchy until it encounters a type definition.
    // If no type is found, an empty string will be returned.
    std::string getAttributeType(const std::string& name);

    // Returns the attribute description string for the given name.
    // This method will walk up the inheritance hierarchy until it encounters a non-empty description.
    std::string getAttributeDescription(const std::string& name);

    /**
     * Function that will be invoked by forEachAttribute.
     *
     * The function will be passed each EntityClassAttribute in turn, along
     * with a bool indicating if this attribute is inherited from a parent
     * entity class.
     */
    using AttributeVisitor = std::function<void(const EntityClassAttribute&, bool)>;

    /**
     * Enumerate the EntityClassAttibutes in turn, including all inherited
     * attributes.
     *
     * \param visitor
     * Function that will be invoked for each EntityClassAttibute.
     *
     * \param editorKeys
     * true if editor keys (those which start with "editor_") should be passed
     * to the visitor, false if they should be skipped.
     */
    void forEachAttribute(AttributeVisitor visitor, bool editorKeys = false);

	/**
	 * Returns true if this entity is of type or inherits from the
	 * given entity class name. className is treated case-sensitively.
	 */
	bool isOfType(const std::string& className);

    void emplaceAttribute(EntityClassAttribute&& attribute);

    // Resets the colour to the value defined in the attributes
    void resetColour();

    void emitChangedSignal()
    {
        if (!_blockChangeSignal)
        {
            _changedSignal.emit();
        }
    }

    void blockChangedSignal(bool block)
    {
        _blockChangeSignal = block;
    }

protected:
    // Initialises this class from the given tokens
    void parseFromTokens(parser::DefTokeniser& tokeniser) override;

    // Clears the structure before parsing
    void onBeginParsing() override;

    // After parsing, inheritance and colour overrides will be resolved
    void onParsingFinished() override;

    void onSyntaxBlockAssigned(const decl::DeclarationBlockSource& block) override;
};

} // namespace scene
