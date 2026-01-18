#include "Transformation.h"

#include <functional>
#include <cmath>

#include "i18n.h"
#include <string>
#include <map>
#include "math/Quaternion.h"
#include "math/curve.h"
#include "iundo.h"
#include "imap.h"
#include "igrid.h"
#include "iorthoview.h"
#include "inamespace.h"
#include "iselection.h"
#include "itextstream.h"
#include "iselectiongroup.h"
#include "icurve.h"
#include "ientity.h"
#include "iarray.h"

#include "scenelib.h"
#include "selectionlib.h"
#include "registry/registry.h"
#include "scene/Clone.h"
#include "scene/EntityNode.h"
#include "map/algorithm/Import.h"
#include "scene/BasicRootNode.h"
#include "debugging/debugging.h"
#include "selection/TransformationVisitors.h"
#include "selection/SceneWalkers.h"
#include "command/ExecutionFailure.h"
#include "parser/Tokeniser.h"
#include "string/convert.h"

#include "string/case_conv.h"

using namespace ui;

namespace selection
{

namespace algorithm
{

namespace
{
	const std::string RKEY_OFFSET_CLONED_OBJECTS = "user/ui/offsetClonedObjects";
}

void rotateSelected(const Quaternion& rotation)
{
	// Perform the rotation according to the current mode
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component)
	{
		GlobalSelectionSystem().foreachSelectedComponent(
			RotateComponentSelected(rotation, GlobalSelectionSystem().getPivot2World().translation()));
	}
	else
	{
		// Cycle through the selections and rotate them
		GlobalSelectionSystem().foreachSelected(RotateSelected(rotation,
			GlobalSelectionSystem().getPivot2World().translation()));
	}

	// Update the views
	SceneChangeNotify();

	GlobalSceneGraph().foreachNode(scene::freezeTransformableNode);
}

// greebo: see header for documentation
void rotateSelected(const Vector3& eulerXYZ)
{
	std::string command("rotateSelectedEulerXYZ: ");
	command += string::to_string(eulerXYZ);
	UndoableCommand undo(command.c_str());

	rotateSelected(Quaternion::createForEulerXYZDegrees(eulerXYZ));
}

void rotateSelectedEulerXYZ(const cmd::ArgumentList& args)
{
	if (args.size() != 1)
	{
		rWarning() << "Usage: RotateSelectedEulerXYZ <eulerAngles:Vector3>" << std::endl;
		return;
	}

	rotateSelected(args[0].getVector3());
}

// greebo: see header for documentation
void scaleSelected(const Vector3& scaleXYZ)
{
	if (fabs(scaleXYZ[0]) > 0.0001f &&
		fabs(scaleXYZ[1]) > 0.0001f &&
		fabs(scaleXYZ[2]) > 0.0001f)
	{
		std::string command("scaleSelected: ");
		command += string::to_string(scaleXYZ);
		UndoableCommand undo(command);

		// Pass the scale to the according traversor
		if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component)
		{
			GlobalSelectionSystem().foreachSelectedComponent(ScaleComponentSelected(scaleXYZ,
				GlobalSelectionSystem().getPivot2World().translation()));
		}
		else
		{
			GlobalSelectionSystem().foreachSelected(ScaleSelected(scaleXYZ,
				GlobalSelectionSystem().getPivot2World().translation()));
		}

		// Update the scene views
		SceneChangeNotify();

		GlobalSceneGraph().foreachNode(scene::freezeTransformableNode);
	}
	else
	{
		throw cmd::ExecutionFailure(_("Cannot scale by zero value."));
	}
}

void scaleSelectedCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 1)
	{
		rWarning() << "Usage: ScaleSelected <scale:Vector3>" << std::endl;
		return;
	}

	scaleSelected(args[0].getVector3());
}

/** greebo: A visitor class cloning the visited selected items.
 *
 * Use it like this:
 * 1) Traverse the scenegraph, this will create clones.
 * 2) The clones get automatically inserted into a temporary container root.
 * 3) Now move the cloneRoot into a temporary namespace to establish the links.
 * 4) Import the nodes into the target namespace
 * 5) Move the nodes into the target scenegraph (using moveClonedNodes())
 */
class SelectionCloner :
	public scene::NodeVisitor
{
public:
	// This maps cloned nodes to the parent nodes they should be inserted in
	typedef std::map<scene::INodePtr, scene::INodePtr> Map;

private:
	// The map which will associate the cloned nodes to their designated parents
	mutable Map _cloned;

	// A container, which temporarily holds the cloned nodes
    std::shared_ptr<scene::BasicRootNode> _cloneRoot;

	// Map group IDs in this selection to new groups
	typedef std::map<std::size_t, ISelectionGroupPtr> GroupMap;
	GroupMap _groupMap;

public:
	SelectionCloner() :
		_cloneRoot(new scene::BasicRootNode)
	{}

	const std::shared_ptr<scene::BasicRootNode>& getCloneRoot()
	{
		return _cloneRoot;
	}

	const Map& getClonedNodes() const
	{
		return _cloned;
	}

	bool pre(const scene::INodePtr& node)
	{
		// Don't clone root items
		if (node->isRoot())
		{
			return true;
		}

		if (Node_isSelected(node))
		{
			// Don't traverse children of cloned nodes
			return false;
		}

		return true;
	}

	void post(const scene::INodePtr& node)
	{
		if (node->isRoot())
		{
			return;
		}

		if (Node_isSelected(node))
		{
			// Clone the current node
			scene::INodePtr clone = scene::cloneNodeIncludingDescendants(node,
				sigc::mem_fun(*this, &SelectionCloner::postProcessClonedNode));

			// Add the cloned node and its parent to the list
			_cloned.emplace(clone, node->getParent());

			// Insert this node in the root
			_cloneRoot->addChildNode(clone);

			// Cloned child nodes are assigned the layers of the source nodes
			// update the layer visibility flags using the layer manager of the source tree
			scene::UpdateNodeVisibilityWalker visibilityUpdater(node->getRootNode()->getLayerManager());
			clone->traverse(visibilityUpdater);
		}
	}

	void postProcessClonedNode(const scene::INodePtr& sourceNode, const scene::INodePtr& clonedNode)
	{
		// Collect and add the group IDs of the source node
		std::shared_ptr<IGroupSelectable> groupSelectable = std::dynamic_pointer_cast<IGroupSelectable>(sourceNode);

		if (groupSelectable)
		{
			auto sourceRoot = sourceNode->getRootNode();
			assert(sourceRoot);

			const IGroupSelectable::GroupIds& groupIds = groupSelectable->getGroupIds();

			// Get the Groups the source node was assigned to, and add the
			// cloned node to the mapped group, one by one, keeping the order intact
			for (std::size_t id : groupIds)
			{
				// Try to insert the ID, ignore if already exists
				// Get a new mapping for the given group ID
				const ISelectionGroupPtr& mappedGroup = getMappedGroup(id, sourceRoot);

				// Assign the new group ID to this clone
				mappedGroup->addNode(clonedNode);
			}
		}
	}

	// Gets the replacement ID for the given group ID
	const ISelectionGroupPtr& getMappedGroup(std::size_t id, const scene::IMapRootNodePtr& sourceRoot)
	{
		auto found = _groupMap.emplace(id, ISelectionGroupPtr());

		if (!found.second)
		{
			// We already covered this ID, return the mapped group
			return found.first->second;
		}

		// Insertion was successful, so we didn't cover this ID yet
		found.first->second = sourceRoot->getSelectionGroupManager().createSelectionGroup();

		return found.first->second;
	}

	// Adds the cloned nodes to their designated parents. Pass TRUE to select the nodes.
	void moveClonedNodes(bool select)
	{
		for (const auto& pair : _cloned)
		{
			// Remove the child from the basic container first
			_cloneRoot->removeChildNode(pair.first);

			// Add the node to its parent
			pair.second->addChildNode(pair.first);

			if (select)
			{
				Node_setSelected(pair.first, true);
			}
		}
	}
};

void cloneSelected(const cmd::ArgumentList& args)
{
	// Check for the correct editing mode (don't clone components)
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component ||
        GlobalMapModule().getEditMode() != IMap::EditMode::Normal)
    {
		return;
	}

	// Get the namespace of the current map
	auto mapRoot = GlobalMapModule().getRoot();
	if (!mapRoot) return; // not map root (this can happen)

	UndoableCommand undo("cloneSelected");

	SelectionCloner cloner;
	GlobalSceneGraph().root()->traverse(cloner);

	// Create a new namespace and move all cloned nodes into it
	INamespacePtr clonedNamespace = GlobalNamespaceFactory().createNamespace();
	assert(clonedNamespace);

	// Move items into the temporary namespace, this will setup the links
	clonedNamespace->connect(cloner.getCloneRoot());

	// Adjust all new names to fit into the existing map namespace
	map::algorithm::prepareNamesForImport(mapRoot, cloner.getCloneRoot());

	// Unselect the current selection
	GlobalSelectionSystem().setSelectedAll(false);

	// Finally, move the cloned nodes to their destination and select them
	cloner.moveClonedNodes(true);

	if (registry::getValue<int>(RKEY_OFFSET_CLONED_OBJECTS) == 1)
	{
		// Move the current selection by one grid unit to the "right" and "downwards"
		nudgeSelected(eNudgeDown);
		nudgeSelected(eNudgeRight);
	}
}

std::vector<scene::INodePtr> createArrayClones(
	const std::vector<scene::INodePtr>& originalSelection,
	int count,
	const std::function<void(int, const scene::INodePtr&)>& applyTransform)
{
	auto mapRoot = GlobalMapModule().getRoot();
	std::vector<scene::INodePtr> allClones;

	if (!mapRoot) return allClones;

	for (int i = 1; i <= count; ++i)
	{
		GlobalSelectionSystem().setSelectedAll(false);

		// Select the original for cloning
		SelectionCloner cloner;
		for (const auto& node : originalSelection)
		{
			Node_setSelected(node, true);
		}

		GlobalSceneGraph().root()->traverse(cloner);
		GlobalSelectionSystem().setSelectedAll(false);

		INamespacePtr clonedNamespace = GlobalNamespaceFactory().createNamespace();
		assert(clonedNamespace);
		clonedNamespace->connect(cloner.getCloneRoot());

		map::algorithm::prepareNamesForImport(mapRoot, cloner.getCloneRoot());
		cloner.moveClonedNodes(false);

		for (const auto& [clonedNode, parentNode] : cloner.getClonedNodes())
		{
			applyTransform(i, clonedNode);
			allClones.push_back(clonedNode);
		}
	}

	for (const auto& node : allClones)
	{
		Node_setSelected(node, true);
	}

	for (const auto& node : originalSelection)
	{
		Node_setSelected(node, true);
	}

	return allClones;
}

void arrayCloneSelectedLine(int count, ArrayOffsetMethod offsetMethod, const Vector3& offset, const Vector3& rotation)
{
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component ||
		GlobalMapModule().getEditMode() != IMap::EditMode::Normal)
	{
		return;
	}

	if (count < 1) return;

	auto mapRoot = GlobalMapModule().getRoot();
	if (!mapRoot) return;

	UndoableCommand undo("arrayCloneSelectedLine");

	// Store the original selection to iterate over
	std::vector<scene::INodePtr> originalSelection;
	GlobalSelectionSystem().foreachSelected([&](const scene::INodePtr& node)
	{
		originalSelection.push_back(node);
	});

	// Calculate the actual offset based on offset method
	Vector3 effectiveOffset = offset;

	if (offsetMethod == ArrayOffsetMethod::Relative)
	{
		// Get bounding box of selection and multiply offset by its extents
		AABB bounds = GlobalSelectionSystem().getWorkZone().bounds;
		Vector3 extents = bounds.getExtents() * 2; // getExtents returns half-extents
		effectiveOffset = Vector3(
			offset.x() * extents.x(),
			offset.y() * extents.y(),
			offset.z() * extents.z()
		);
	}
	else if (offsetMethod == ArrayOffsetMethod::Endpoint)
	{
		// Offset represents the total distance, divide by count
		effectiveOffset = offset / static_cast<double>(count);
	}

	createArrayClones(originalSelection, count, [&](int i, const scene::INodePtr& clonedNode)
	{
		Vector3 currentOffset = effectiveOffset * static_cast<double>(i);
		Vector3 currentRotation = rotation * static_cast<double>(i);

		ITransformablePtr transformable = scene::node_cast<ITransformable>(clonedNode);
		if (transformable)
		{
			transformable->setType(TRANSFORM_PRIMITIVE);
			transformable->setTranslation(currentOffset);
			transformable->freezeTransform();

			// Apply rotation if any
			if (currentRotation.getLengthSquared() > 0)
			{
				Quaternion rot = Quaternion::createForEulerXYZDegrees(currentRotation);
				transformable->setType(TRANSFORM_PRIMITIVE);
				transformable->setRotation(rot);
				transformable->freezeTransform();
			}
		}
	});
}

void arrayCloneSelectedLineCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 4)
	{
		rWarning() << "Usage: ArrayCloneSelectionLine <count:int> <offsetMethod:int> <offset:Vector3> <rotation:Vector3>" << std::endl;
		return;
	}

	int count = args[0].getInt();
	ArrayOffsetMethod offsetMethod = static_cast<ArrayOffsetMethod>(args[1].getInt());
	Vector3 offset = args[2].getVector3();
	Vector3 rotation = args[3].getVector3();

	arrayCloneSelectedLine(count, offsetMethod, offset, rotation);
}

void arrayCloneSelectedCircle(int count, float radius, float startAngle, float endAngle, bool rotateToCenter)
{
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component ||
		GlobalMapModule().getEditMode() != IMap::EditMode::Normal)
	{
		return;
	}

	if (count < 1) return;

	auto mapRoot = GlobalMapModule().getRoot();
	if (!mapRoot) return;

	UndoableCommand undo("arrayCloneSelectedCircle");

	// Store the original selection to iterate over
	std::vector<scene::INodePtr> originalSelection;
	GlobalSelectionSystem().foreachSelected([&](const scene::INodePtr& node)
	{
		originalSelection.push_back(node);
	});

	// Get center of selection as the circle center
	AABB bounds = GlobalSelectionSystem().getWorkZone().bounds;
	Vector3 center = bounds.getOrigin();

	// Convert angles to radians
	float startRad = degrees_to_radians(startAngle);
	float endRad = degrees_to_radians(endAngle);
	float angleRange = endRad - startRad;

	// Distribute copies evenly around the arc
	// If full circle (360 degrees), don't place copy at both start and end
	bool fullCircle = std::abs(endAngle - startAngle) >= 360.0f;
	int divisor = fullCircle ? count : (count > 1 ? count - 1 : 1);

	createArrayClones(originalSelection, count, [&](int i, const scene::INodePtr& clonedNode)
	{
		// Calculate angle for this copy (i starts at 1, so subtract 1 for 0-based index)
		float t = static_cast<float>(i - 1) / static_cast<float>(divisor);
		float angle = startRad + angleRange * t;

		// Calculate position on circle (in XY plane)
		Vector3 offset(
			radius * cos(angle),
			radius * sin(angle),
			0
		);

		ITransformablePtr transformable = scene::node_cast<ITransformable>(clonedNode);
		if (transformable)
		{
			transformable->setType(TRANSFORM_PRIMITIVE);
			transformable->setTranslation(offset);
			transformable->freezeTransform();

			if (rotateToCenter)
			{
				float rotAngle = angle + static_cast<float>(math::PI);
				Quaternion rot = Quaternion::createForZ(rotAngle);
				transformable->setType(TRANSFORM_PRIMITIVE);
				transformable->setRotation(rot);
				transformable->freezeTransform();
			}
		}
	});
}

void arrayCloneSelectedCircleCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 5)
	{
		rWarning() << "Usage: ArrayCloneSelectionCircle <count:int> <radius:float> <startAngle:float> <endAngle:float> <rotateToCenter:int>" << std::endl;
		return;
	}

	int count = args[0].getInt();
	float radius = static_cast<float>(args[1].getDouble());
	float startAngle = static_cast<float>(args[2].getDouble());
	float endAngle = static_cast<float>(args[3].getDouble());
	bool rotateToCenter = args[4].getInt() != 0;

	arrayCloneSelectedCircle(count, radius, startAngle, endAngle, rotateToCenter);
}

void arrayCloneSelectedSpline(int count, bool alignToSpline)
{
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component ||
		GlobalMapModule().getEditMode() != IMap::EditMode::Normal)
	{
		return;
	}

	if (count < 1) return;

	auto mapRoot = GlobalMapModule().getRoot();
	if (!mapRoot) return;

	// Find the curve entity and other selected nodes
	scene::INodePtr curveNode;
	std::vector<scene::INodePtr> nodesToClone;

	GlobalSelectionSystem().foreachSelected([&](const scene::INodePtr& node)
	{
		CurveNodePtr curve = Node_getCurve(node);
		if (curve && !curve->hasEmptyCurve())
		{
			if (!curveNode)
			{
				curveNode = node;
			}
		}
		else
		{
			nodesToClone.push_back(node);
		}
	});

	if (!curveNode)
	{
		throw cmd::ExecutionFailure(_("Cannot create spline array: No curve entity selected.\nSelect a curve entity along with objects to clone."));
	}

	if (nodesToClone.empty())
	{
		throw cmd::ExecutionFailure(_("Cannot create spline array: No objects selected to clone.\nSelect objects along with the curve entity."));
	}

	// Get the entity world transform to transform control points to world space
	Matrix4 curveTransform = curveNode->localToWorld();

	// We need to get the control points from the entity spawnargs
	Entity* entity = Node_getEntity(curveNode);
	if (!entity)
	{
		throw cmd::ExecutionFailure(_("Cannot create spline array: Could not access curve entity data."));
	}

	// Try to get curve data from entity spawnargs
	std::string curveKey = entity->getKeyValue("curve_CatmullRomSpline");
	if (curveKey.empty())
	{
		curveKey = entity->getKeyValue("curve_Nurbs");
	}

	if (curveKey.empty())
	{
		throw cmd::ExecutionFailure(_("Cannot create spline array: No curve data found on entity."));
	}

	// Parse the curve controls points
	ControlPoints controlPoints;
	parser::BasicStringTokeniser tokeniser(curveKey, " ");
	try
	{
		std::size_t size = string::convert<int>(tokeniser.nextToken());
		if (size < 2)
		{
			throw cmd::ExecutionFailure(_("Cannot create spline array: Curve has less than 2 control points."));
		}

		tokeniser.assertNextToken("(");

		for (std::size_t i = 0; i < size; ++i)
		{
			Vector3 point;
			point.x() = string::convert<float>(tokeniser.nextToken());
			point.y() = string::convert<float>(tokeniser.nextToken());
			point.z() = string::convert<float>(tokeniser.nextToken());

			// Transform to world space
			point = curveTransform.transformPoint(point);
			controlPoints.push_back(point);
		}
	}
	catch (const cmd::ExecutionFailure&)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw cmd::ExecutionFailure(std::string(_("Cannot create spline array: Failed to parse curve - ")) + e.what());
	}

	AABB objectsBounds;
	for (const auto& node : nodesToClone)
	{
		objectsBounds.includeAABB(node->worldAABB());
	}
	Vector3 objectsCenter = objectsBounds.getOrigin();

	UndoableCommand undo("arrayCloneSelectedSpline");

	createArrayClones(nodesToClone, count, [&](int i, const scene::INodePtr& clonedNode)
	{
		// Calculate t parameter (0 to 1) along the spline
		double t = (count > 1) ? static_cast<double>(i - 1) / static_cast<double>(count - 1) : 0.0;

		Vector3 position = CatmullRom_evaluate(controlPoints, t);
		Vector3 offset = position - objectsCenter;

		ITransformablePtr transformable = scene::node_cast<ITransformable>(clonedNode);
		if (transformable)
		{
			transformable->setType(TRANSFORM_PRIMITIVE);
			transformable->setTranslation(offset);
			transformable->freezeTransform();

			if (alignToSpline && controlPoints.size() >= 2)
			{
				Vector3 tangent;
				float epsilon = 0.01f;
				if (t + epsilon <= 1.0f)
				{
					Vector3 nextPos = CatmullRom_evaluate(controlPoints, t + epsilon);
					tangent = (nextPos - position).getNormalised();
				}
				else if (t - epsilon >= 0.0f)
				{
					Vector3 prevPos = CatmullRom_evaluate(controlPoints, t - epsilon);
					tangent = (position - prevPos).getNormalised();
				}
				else
				{
					// forward
					tangent = Vector3(1, 0, 0);
				}

				Vector3 forward(1, 0, 0);
				if (tangent.getLengthSquared() > 0.001)
				{
					Quaternion rot = Quaternion::createForUnitVectors(forward, tangent);
					transformable->setType(TRANSFORM_PRIMITIVE);
					transformable->setRotation(rot);
					transformable->freezeTransform();
				}
			}
		}
	});
}

void arrayCloneSelectedSplineCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 2)
	{
		rWarning() << "Usage: ArrayCloneSelectionSpline <count:int> <alignToSpline:int>" << std::endl;
		return;
	}

	int count = args[0].getInt();
	bool alignToSpline = args[1].getInt() != 0;

	arrayCloneSelectedSpline(count, alignToSpline);
}

struct AxisBase
{
	Vector3 x;
	Vector3 y;
	Vector3 z;

	AxisBase(const Vector3& x_, const Vector3& y_, const Vector3& z_) :
		x(x_),
		y(y_),
		z(z_)
	{}
};

AxisBase AxisBase_forViewType(OrthoOrientation viewtype)
{
	switch(viewtype)
	{
	case OrthoOrientation::XY:
		return AxisBase(g_vector3_axis_x, g_vector3_axis_y, g_vector3_axis_z);
	case OrthoOrientation::XZ:
		return AxisBase(g_vector3_axis_x, g_vector3_axis_z, g_vector3_axis_y);
	case OrthoOrientation::YZ:
		return AxisBase(g_vector3_axis_y, g_vector3_axis_z, g_vector3_axis_x);
	}

	ERROR_MESSAGE("invalid viewtype");
	return AxisBase(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0));
}

Vector3 AxisBase_axisForDirection(const AxisBase& axes, ENudgeDirection direction)
{
	switch (direction)
	{
	case eNudgeLeft:
		return -axes.x;
	case eNudgeUp:
		return axes.y;
	case eNudgeRight:
		return axes.x;
	case eNudgeDown:
		return -axes.y;
	}

	ERROR_MESSAGE("invalid direction");
	return Vector3(0, 0, 0);
}

void translateSelected(const Vector3& translation)
{
	// Apply the transformation and freeze the changes
	if (GlobalSelectionSystem().getSelectionMode() == SelectionMode::Component)
	{
		GlobalSelectionSystem().foreachSelectedComponent(TranslateComponentSelected(translation));
	}
	else
	{
		// Cycle through the selected items and apply the translation
		GlobalSelectionSystem().foreachSelected(TranslateSelected(translation));
	}

	// Update the scene so that the changes are made visible
	SceneChangeNotify();

	GlobalSceneGraph().foreachNode(scene::freezeTransformableNode);
}

// Specialised overload, called by the general nudgeSelected() routine
void nudgeSelected(ENudgeDirection direction, float amount, OrthoOrientation viewtype)
{
	AxisBase axes(AxisBase_forViewType(viewtype));

	//Vector3 view_direction(-axes.z);
	Vector3 nudge(AxisBase_axisForDirection(axes, direction) * amount);

	if (GlobalSelectionSystem().getActiveManipulatorType() == selection::IManipulator::Translate ||
        GlobalSelectionSystem().getActiveManipulatorType() == selection::IManipulator::Drag ||
        GlobalSelectionSystem().getActiveManipulatorType() == selection::IManipulator::Clip)
    {
        translateSelected(nudge);

        // In clip mode, update the clipping plane
        if (GlobalSelectionSystem().getActiveManipulatorType() == selection::IManipulator::Clip)
        {
            GlobalClipper().update();
        }
    }
}

void nudgeSelected(ENudgeDirection direction)
{
	nudgeSelected(direction, GlobalGrid().getGridSize(), GlobalOrthoViewManager().getActiveViewType());
}

void nudgeSelectedCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 1)
	{
		rMessage() << "Usage: nudgeSelected [up|down|left|right]" << std::endl;
		return;
	}

	UndoableCommand undo("nudgeSelected");

	std::string arg = string::to_lower_copy(args[0].getString());

	if (arg == "up") {
		nudgeSelected(eNudgeUp);
	}
	else if (arg == "down") {
		nudgeSelected(eNudgeDown);
	}
	else if (arg == "left") {
		nudgeSelected(eNudgeLeft);
	}
	else if (arg == "right") {
		nudgeSelected(eNudgeRight);
	}
	else {
		// Invalid argument
		rMessage() << "Usage: nudgeSelected [up|down|left|right]" << std::endl;
		return;
	}
}

void nudgeByAxis(int nDim, float fNudge)
{
	Vector3 translate(0, 0, 0);
	translate[nDim] = fNudge;

	translateSelected(translate);
}

void moveSelectedAlongZ(float amount)
{
	std::ostringstream command;
	command << "nudgeSelected -axis z -amount " << amount;
	UndoableCommand undo(command.str());

	nudgeByAxis(2, amount);
}

void moveSelectedVerticallyCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 1)
	{
		rMessage() << "Usage: moveSelectionVertically [up|down]" << std::endl;
		return;
	}

	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("moveSelectionVertically");

	std::string arg = string::to_lower_copy(args[0].getString());

	if (arg == "up")
	{
		moveSelectedAlongZ(GlobalGrid().getGridSize());
	}
	else if (arg == "down")
	{
		moveSelectedAlongZ(-GlobalGrid().getGridSize());
	}
	else
	{
		// Invalid argument
		rMessage() << "Usage: moveSelectionVertically [up|down]" << std::endl;
		return;
	}
}

void moveSelectedCmd(const cmd::ArgumentList& args)
{
	if (args.size() != 1)
	{
		rMessage() << "Usage: moveSelection <vector>" << std::endl;
		return;
	}

	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("moveSelection");

	auto translation = args[0].getVector3();
    translateSelected(translation);
}

enum axis_t
{
	eAxisX = 0,
	eAxisY = 1,
	eAxisZ = 2,
};

enum sign_t
{
	eSignPositive = 1,
	eSignNegative = -1,
};

inline Quaternion quaternion_for_axis90(axis_t axis, sign_t sign)
{
	switch(axis)
	{
	case eAxisX:
		if (sign == eSignPositive)
		{
			return Quaternion(c_half_sqrt2, 0, 0, c_half_sqrt2);
		}
		else
		{
			return Quaternion(-c_half_sqrt2, 0, 0, c_half_sqrt2);
		}
	case eAxisY:
		if(sign == eSignPositive)
		{
			return Quaternion(0, c_half_sqrt2, 0, c_half_sqrt2);
		}
		else
		{
			return Quaternion(0, -c_half_sqrt2, 0, c_half_sqrt2);
		}
	default://case eAxisZ:
		if(sign == eSignPositive)
		{
			return Quaternion(0, 0, c_half_sqrt2, c_half_sqrt2);
		}
		else
		{
			return Quaternion(0, 0, -c_half_sqrt2, c_half_sqrt2);
		}
	}
}

void rotateSelectionX(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("rotateSelected -axis x -angle -90");
    rotateSelected(quaternion_for_axis90(eAxisX, eSignNegative));
}

void rotateSelectionY(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("rotateSelected -axis y -angle 90");
    rotateSelected(quaternion_for_axis90(eAxisY, eSignPositive));
}

void rotateSelectionZ(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("rotateSelected -axis z -angle -90");
    rotateSelected(quaternion_for_axis90(eAxisZ, eSignNegative));
}

void mirrorSelection(int axis)
{
	Vector3 flip(1, 1, 1);
	flip[axis] = -1;

	scaleSelected(flip);
}

void mirrorSelectionX(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("mirrorSelected -axis x");
	mirrorSelection(0);
}

void mirrorSelectionY(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("mirrorSelected -axis y");
	mirrorSelection(1);
}

void mirrorSelectionZ(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().countSelected() == 0)
	{
		rMessage() << "Nothing selected." << std::endl;
		return;
	}

	UndoableCommand undo("mirrorSelected -axis z");
	mirrorSelection(2);
}

} // namespace algorithm

} // namespace selection
