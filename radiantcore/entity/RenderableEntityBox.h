#pragma once

#include "render/RenderableBox.h"

class EntityNode;

namespace entity
{

/// RenderableBox subclass for entity nodes
class RenderableEntityBox final: public render::RenderableBox
{
    const EntityNode& _entity;

public:
    RenderableEntityBox(const EntityNode& entity, const AABB* bounds, const Vector3& worldPos);

protected:
    Vector4 getVertexColour() override;
};

}
