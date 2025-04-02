#ifndef BOUNDED_H_
#define BOUNDED_H_

#include <memory>

/* FOWARD DECLS */
class AABB;

/// Interface for bounded objects, which have a local AABB.
class Bounded
{
public:
    virtual ~Bounded() {}

    /// Return the local AABB for this object.
    virtual AABB localAABB() const = 0;
};
typedef std::shared_ptr<Bounded> BoundedPtr;

#endif /*BOUNDED_H_*/
