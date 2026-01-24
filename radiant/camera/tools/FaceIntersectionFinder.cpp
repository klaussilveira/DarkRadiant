#include "FaceIntersectionFinder.h"

#include "ifilter.h"
#include "ibrush.h"
#include "scenelib.h"
#include "math/Vector4.h"

#include <cmath>

namespace ui
{

FaceIntersectionFinder::FaceIntersectionFinder(SelectionTest& test, const Matrix4& viewProjection) :
    _selectionTest(test)
{
    computeWorldRay(viewProjection, _worldRayOrigin, _worldRayDirection);
}

void FaceIntersectionFinder::computeWorldRay(const Matrix4& viewProjection,
                                              Vector3& outOrigin, Vector3& outDirection)
{
    Matrix4 invViewProj = viewProjection.getFullInverse();

    // Use getProjected() for proper perspective divide (not getVector3())
    Vector4 nearClip = invViewProj.transform(Vector4(0, 0, -1, 1));
    Vector4 farClip = invViewProj.transform(Vector4(0, 0, 1, 1));

    outOrigin = nearClip.getProjected();
    outDirection = (farClip.getProjected() - outOrigin).getNormalised();
}

bool FaceIntersectionFinder::pre(const scene::INodePtr& node)
{
    if (!node->visible())
    {
        return false;
    }

    if (Node_isEntity(node))
    {
        return scene::hasChildPrimitives(node);
    }

    IBrush* brush = Node_getIBrush(node);

    if (brush != nullptr)
    {
        _selectionTest.BeginMesh(node->localToWorld());

        for (std::size_t i = 0; i < brush->getNumFaces(); ++i)
        {
            const IFace& face = brush->getFace(i);

            if (!GlobalFilterSystem().isVisible(FilterType::TEXTURE, face.getShader()))
            {
                continue;
            }

            const IWinding& winding = face.getWinding();

            if (winding.empty())
            {
                continue;
            }

            SelectionIntersection intersection;
            _selectionTest.TestPolygon(
                VertexPointer(&winding.front().vertex, sizeof(WindingVertex)),
                winding.size(),
                intersection
            );

            if (intersection.isValid() && intersection.isCloserThan(_bestIntersection))
            {
                _bestIntersection = intersection;
                _hasResult = true;
                _bestPlane = face.getPlane3();
                _bestNode = node;
            }
        }
    }

    return true;
}

FaceIntersection FaceIntersectionFinder::getResult() const
{
    FaceIntersection result;

    if (!_hasResult)
    {
        return result;
    }

    const Matrix4& localToWorld = _bestNode->localToWorld();

    // Transform plane to world space
    Vector3 localNormal = _bestPlane.normal();
    Vector3 localPointOnPlane = localNormal * _bestPlane.dist();
    Vector3 worldPointOnPlane = localToWorld.transformPoint(localPointOnPlane);
    Vector3 worldNormal = localToWorld.transformDirection(localNormal).getNormalised();
    double worldDist = worldNormal.dot(worldPointOnPlane);

    // Ray-plane intersection
    double denom = worldNormal.dot(_worldRayDirection);
    Vector3 worldIntersection;

    if (std::abs(denom) > 0.0001)
    {
        double t = (worldDist - worldNormal.dot(_worldRayOrigin)) / denom;
        worldIntersection = _worldRayOrigin + _worldRayDirection * t;
    }
    else
    {
        worldIntersection = worldPointOnPlane;
    }

    result.valid = true;
    result.point = worldIntersection;
    result.normal = worldNormal;
    result.node = _bestNode;

    return result;
}

} // namespace ui
