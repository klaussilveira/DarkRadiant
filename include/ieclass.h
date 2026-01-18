/**
 * \defgroup eclass Entity class manager
 * \file ieclass.h
 * \brief Entity Class definition loader API.
 * \ingroup eclass
 */
#pragma once

#include "ideclmanager.h"
#include "imodule.h"

#include <map>
#include <memory>
#include <sigc++/signal.h>

#include "scene/EntityClass.h"

/* FORWARD DECLS */

class Shader;
typedef std::shared_ptr<Shader> ShaderPtr;
class AABB;

/**
 * Structure ontains the information of a model {} block as defined in a
 * Doom3 .def file.
 *
 * \ingroup eclass
 */
class IModelDef :
    public decl::IDeclaration
{
public:
    using Ptr = std::shared_ptr<IModelDef>;

    // The def this model is inheriting from (empty if there's no parent)
    virtual const Ptr& getParent() = 0;

    // The MD5 mesh used by this modelDef
    virtual const std::string& getMesh() = 0;

    // The named skin
    virtual const std::string& getSkin() = 0;

    // The md5anim file name for the given anim key (e.g. "idle" or "af_pose")
    virtual std::string getAnim(const std::string& animKey) = 0;

    // Returns a dictionary of all the animations declared on this model def
    using Anims = std::map<std::string, std::string>;
    virtual const Anims& getAnims() = 0;
};

/**
 * EntityClass visitor interface.
 *
 * \ingroup eclass
 */
class EntityClassVisitor
{
public:
    virtual ~EntityClassVisitor() {}
    virtual void visit(const scene::EntityClass::Ptr& eclass) = 0;
};

constexpr const char* const MODULE_ECLASSMANAGER("EntityClassManager");

/**
 * EntityClassManager interface. The entity class manager is responsible for
 * maintaining a list of available entity classes which the EntityCreator can
 * insert into a map.
 *
 * \ingroup eclass
 */
class IEntityClassManager :
    public RegisterableModule
{
public:
    /**
     * Return the scene::EntityClass corresponding to the given name, creating it if
     * necessary. If it is created, the has_brushes parameter will be used to
     * determine whether the new entity class should be brush-based or not.
     *
     * @deprecated
     * Use findClass() instead.
     */
    virtual scene::EntityClass::Ptr findOrInsert(const std::string& name,
                                         bool has_brushes) = 0;

    /**
     * Lookup an entity class by name. If the class is not found, a null pointer
     * is returned.
     *
     * @param name
     * Name of the entity class to look up.
     */
    virtual scene::EntityClass::Ptr findClass(const std::string& name) = 0;

    /**
     * Iterate over all entity defs using the given visitor.
     */
    virtual void forEachEntityClass(EntityClassVisitor& visitor) = 0;

    // Iterate over all entityDefs using the given function object
    virtual void forEachEntityClass(const std::function<void(const scene::EntityClass::Ptr&)>& functor) = 0;

    /**
     * greebo: This reloads the entityDefs and modelDefs from all files. Does not
     * change the scenegraph, only the contents of the EClass objects are
     * re-parsed. All scene::EntityClass::Ptrs remain valid, no entityDefs are removed.
     *
     * Note: This is NOT the same as unrealise + realise
     */
    virtual void reloadDefs() = 0;

    /**
     * greebo: Finds the model def with the given name. Might return NULL if not found.
     */
    virtual IModelDef::Ptr findModel(const std::string& name) = 0;

    /**
     * Iterate over each ModelDef using the given function object.
     */
    virtual void forEachModelDef(const std::function<void(const IModelDef::Ptr&)>& functor) = 0;
};

/**
 * Return the global EntityClassManager to the application.
 *
 * \ingroup eclass
 */
inline IEntityClassManager& GlobalEntityClassManager()
{
    static module::InstanceReference<IEntityClassManager> _reference(MODULE_ECLASSMANAGER);
    return _reference;
}
