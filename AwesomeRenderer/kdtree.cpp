#include "stdafx.h"
#include "awesomerenderer.h"
#include "kdtree.h"
#include "kdtreenode.h"
#include "treeelement.h"

#define NEW(type, args) new (Allocate<type>()) type(args)

using namespace AwesomeRenderer;
const float KDTree::TRAVERSAL_COST = 20.0f;
const float KDTree::INTERSECTION_COST = 1.0f;
const float KDTree::POSITION_EPSILON = 1e-7f;
const float KDTree::PREVENT_SPLIT_MAX_OVERFLOW = 5.0f;
const float KDTree::PREVENT_SPLIT_OVERLAPPING_ELEMENTS = 0.3f;

KDTree::KDTree(uint32_t maxElementsPerLeaf, uint32_t maxDepth) :
	maxElementsPerLeaf(maxElementsPerLeaf), maxDepth(maxDepth)
{
	rootNode = NEW(KDTreeNode, NEW(KDTreeNode::ElementList));
}

KDTree::~KDTree()
{
	rootNode->~KDTreeNode();

	_aligned_free(rootNode);
}

void KDTree::Optimize(const AABB& bounds)
{
	this->bounds = bounds;

	Optimize(rootNode, bounds);
}

void KDTree::Analyze() const
{
	uint32_t leaves = 0;
	uint32_t emptyLeaves = 0;
	uint32_t crampedLeaves = 0;
	float leafFillPercentage = 0.0f;

	std::vector<const KDTreeNode*> nodesLeft;

	nodesLeft.push_back(rootNode);

	while (!nodesLeft.empty())
	{
		const KDTreeNode* node = nodesLeft.back();
		nodesLeft.pop_back();

		if (node->IsLeaf())
		{
			++leaves;

			uint32_t elements = node->GetElements().size();
			if (elements > 0)
				leafFillPercentage += elements / (float)maxElementsPerLeaf;
			else
				++emptyLeaves;

			if (elements > maxElementsPerLeaf)
				++crampedLeaves;
		}
		else
		{
			nodesLeft.push_back(node->GetUpperNode());
			nodesLeft.push_back(node->GetLowerNode());
		}
	}

	leafFillPercentage /= (leaves - emptyLeaves);

	printf("[KDTree]: Optimized with %u leaves, %u empty, %u cramped. Avg. fill percentage: %.0f%%\n",
		leaves, emptyLeaves, crampedLeaves, (leafFillPercentage * 100));
}


bool KDTree::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	float tMax, tMin;
	if (!bounds.IntersectRay(ray, tMin, tMax))
		return false;

	if (tMin > maxDistance)
		return false;

	return IntersectRay(rootNode, ray, hitInfo, maxDistance, tMin, tMax);
}

void KDTree::Optimize(KDTreeNode* node, const AABB& bounds, int depth)
{
	assert(node->IsLeaf());

	KDTreeNode::ElementList& elements = node->GetElements();

	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (elements.size() <= maxElementsPerLeaf || depth >= maxDepth)
		return;

	// If the splitting algorithm can't find a split point which whill improve the tree, leave it as is
	if (!SplitFast(node, bounds))
		return;

	KDTreeNode::ElementList* upperElements = NEW(KDTreeNode::ElementList);
	KDTreeNode::ElementList* lowerElements = NEW(KDTreeNode::ElementList);

	KDTreeNode* nodes = Allocate<KDTreeNode>(2);
	
	KDTreeNode* upperNode = new (nodes) KDTreeNode(upperElements, node);
	KDTreeNode* lowerNode = new (nodes + 1) KDTreeNode(lowerElements, node);
	
	int axis = node->GetAxis();

	float splitPoint = node->GetSplitPoint();
	
	// Relocate all objects in this leaf to the child nodes they intersect
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		const Primitive& primitive = (*it)->GetPrimitive();

		// Determine which side of the plane this object is
		int side = primitive.SideOfAAPlane(axis, splitPoint);

		if (side >= 0)
			upperElements->push_back(*it);

		if (side <= 0)
			lowerElements->push_back(*it);
	}

	node->SetLeaf(false);
	node->SetUpperNode(upperNode);

	// Clear object list since this is no longer a leaf
	Free(&elements);

	// Try to optimize child nodes
	AABB upperBounds, lowerBounds;

	CalculateBounds(bounds, axis, splitPoint, upperBounds, lowerBounds);
	Optimize(upperNode, upperBounds, depth + 1);
	Optimize(lowerNode, lowerBounds, depth + 1);
}

bool KDTree::SplitSAH(KDTreeNode* node, const AABB& bounds)
{
	KDTreeNode::ElementList& elements = node->GetElements();
	int axis = node->GetAxis();

	// Initialize a potentional position list with two positions per object
	std::vector<float> splitPositions(elements.size() * 2);

	// Iterate through all objects in this leaf
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		// Calculate axis aligned boundaries for this shape
		AABB objectBounds;
		(*it)->GetPrimitive().CalculateBounds(objectBounds);

		// Save the minimum and maximum position for the bounds
		splitPositions.push_back(objectBounds.Min()[axis] - KDTree::POSITION_EPSILON);
		splitPositions.push_back(objectBounds.Max()[axis] + KDTree::POSITION_EPSILON);
	}

	// Sort split positions on the axis
	std::sort(splitPositions.begin(), splitPositions.end());

	float lowestCost = FLT_MAX;
	float lowestSplitPoint;

	// Normal vector for the split plane we create
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;

	// Copy the object list to keep count of which objects are potentionally above the current split plane
	std::vector<const TreeElement*>& elementsAbove = temporaryElementList;
	elementsAbove.insert(elementsAbove.end(), elements.begin(), elements.end());

	uint32_t elementCount = elementsAbove.size();

	const Vector3& max = bounds.Max();
	const Vector3& min = bounds.Min();

	// Calculate the area or the two constant axes
	int lowerAxis = axis > 0 ? axis - 1 : 2;
	int upperAxis = (axis + 1) % 3;
	float width = max[lowerAxis] - min[lowerAxis];
	float height = max[upperAxis] - min[upperAxis];
	float baseArea = width * height;

	float rootArea = bounds.CalculateSurfaceArea();
	float dontSplitCost = KDTree::INTERSECTION_COST * elements.size();

	// Iterate through the selected split positions
	for (uint32_t pointIdx = 0, pointCount = splitPositions.size(); pointIdx < pointCount; ++pointIdx)
	{
		float point = splitPositions[pointIdx];

		int upperObjectCount = 0;
		int lowerObjectCount = elements.size() - elementCount; // Start with the number of objects we already know that are lower than this

															   // Iterate through all objects to count how many objects fall on each side of the split plane
		for (uint32_t elementIdx = 0; elementIdx < elementCount; )
		{
			const Primitive& primitive = elementsAbove[elementIdx]->GetPrimitive();

			// Determine which side of the plane this object is
			int side = primitive.SideOfAAPlane(axis, point);

			if (side >= 0)
				++upperObjectCount;

			if (side <= 0)
				++lowerObjectCount;

			// Objects located below this split plane don't need to be checked the next iteration
			if (side < 0)
			{
				// Swap the element with the last element in the vector.
				Util::Swap(elementsAbove[elementIdx], elementsAbove[elementCount - 1]);
				--elementCount;
			}
			else
				++elementIdx;
		}

		// Calculate area for upper and lower nodes
		float upperArea = 2 * (baseArea + (max[axis] - point) * width + (max[axis] - point) * height);
		float lowerArea = 2 * (baseArea + (point - min[axis]) * width + (point - min[axis]) * height);

		// Calculate final cost for splitting at this point
		float cost = KDTree::TRAVERSAL_COST + (KDTree::INTERSECTION_COST * (upperArea / rootArea) * upperObjectCount) +
			KDTree::TRAVERSAL_COST + (KDTree::INTERSECTION_COST * (lowerArea / rootArea) * lowerObjectCount);

		// Do we have a new lowest cost split point?
		if (cost < lowestCost)
		{
			lowestCost = cost;
			lowestSplitPoint = point;
		}
	}

	elementsAbove.clear();

	node->SetSplitPoint(lowestSplitPoint);
	return lowestCost < dontSplitCost;
}

void KDTree::SplitMode(KDTreeNode* node, const AABB& bounds)
{
	KDTreeNode::ElementList& elements = node->GetElements();
	int axis = node->GetAxis();

	float median = 0.0f;

	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		AABB elementBounds;
		(*it)->GetPrimitive().CalculateBounds(elementBounds);

		float center = (elementBounds.Min()[axis] + elementBounds.Max()[axis]) / 2;
		median += center;
	}

	median /= elements.size();

	float splitPoint;

	// Find the element closest to the median
	float closestDistance = FLT_MAX;
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		AABB elementBounds;
		(*it)->GetPrimitive().CalculateBounds(elementBounds);

		float min = elementBounds.Min()[axis];
		float max = elementBounds.Max()[axis];

		float dMin = fabs(median - min);
		float dMax = fabs(median - max);

		if (dMin < closestDistance)
		{
			splitPoint = min - KDTree::POSITION_EPSILON;
			closestDistance = dMin;
		}

		if (dMax < closestDistance)
		{
			splitPoint = max + KDTree::POSITION_EPSILON;
			closestDistance = dMax;
		}
	}

	node->SetSplitPoint(splitPoint);
}

bool KDTree::SplitFast(KDTreeNode* node, const AABB& bounds)
{
	KDTreeNode::ElementList& elements = node->GetElements();
	int axis = node->GetAxis();

	float min = bounds.Min()[axis];
	float max = bounds.Max()[axis];

	float splitPoint = (min + max) / 2.0f;
	node->SetSplitPoint(splitPoint);
	
	// Before performing the real split, check if it actually improves our tree
	uint32_t overlappingElements = 0;
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		const Primitive& primitive = (*it)->GetPrimitive();

		// Determine which side of the plane this object is
		int side = primitive.SideOfAAPlane(axis, splitPoint);

		if (side == 0)
			++overlappingElements;
	}

	// Don't perform the split if elements end up in both nodes
	if (elements.size() < maxElementsPerLeaf * KDTree::PREVENT_SPLIT_MAX_OVERFLOW &&
		overlappingElements / (float)elements.size() >= KDTree::PREVENT_SPLIT_OVERLAPPING_ELEMENTS)
		return false;

	return true;
}

void KDTree::CalculateBounds(const AABB& bounds, int axis, float splitPoint, AABB& upper, AABB& lower)
{
	const Vector3& min = bounds.Min();
	const Vector3& max = bounds.Max();

	Vector3 minSplit = min;
	Vector3 maxSplit = max;

	minSplit[axis] = splitPoint;
	maxSplit[axis] = splitPoint;

	lower.Initialize(min, maxSplit);
	upper.Initialize(minSplit, max);
}

bool KDTree::IntersectRay(KDTreeNode* node, const Ray& ray, RaycastHit& hitInfo, float maxDistance, float tMin, float tMax) const
{
	// The current node is a leaf node, this means we can check its contents
	if (node->IsLeaf())
	{
		float closestDistance = maxDistance;
		bool hit = false;

		KDTreeNode::ElementList& elements = node->GetElements();
		for (uint32_t elementIdx = 0, elementCount = elements.size(); elementIdx < elementCount; ++elementIdx)
		{
			const Shape& shape = elements[elementIdx]->GetShape();

			// Perform the ray-triangle intersection
			RaycastHit shapeHitInfo;
			if (!shape.IntersectRay(ray, shapeHitInfo, closestDistance))
				continue;

			closestDistance = shapeHitInfo.distance;

			hitInfo = shapeHitInfo;
			hitInfo.element = elements[elementIdx];

			hit = true;
		}

		return hit;
	}
	else
	{
		int axis = node->GetAxis();
		float splitPoint = node->GetSplitPoint();

		KDTreeNode* nearNode;
		KDTreeNode* farNode;

		if (ray.origin[axis] < splitPoint)
		{
			nearNode = node->GetLowerNode();
			farNode = node->GetUpperNode();
		}
		else
		{
			nearNode = node->GetUpperNode();
			farNode = node->GetLowerNode();
		}

		float tSplit = (splitPoint - ray.origin[axis]) * ray.invDirection[axis];

		if (tSplit > tMax)
			return IntersectRay(nearNode, ray, hitInfo, maxDistance, tMin, tMax);

		if (tSplit >= tMin)
		{
			if (tSplit > 0)
			{
				if (IntersectRay(nearNode, ray, hitInfo, std::min(maxDistance, tSplit), tMin, tMax))
				{
					// We should only return this intersection if the intersection point is inside the node...
					if (hitInfo.distance <= tSplit)
						return true;
				}

				return IntersectRay(farNode, ray, hitInfo, maxDistance, tMin, tMax);
			}

			return IntersectRay(nearNode, ray, hitInfo, maxDistance, tMin, tMax);
		}
		else
		{
			if (tSplit > 0)
				return IntersectRay(farNode, ray, hitInfo, maxDistance, tMin, tMax);

			if (tSplit < 0)
				return IntersectRay(nearNode, ray, hitInfo, maxDistance, tMin, tMax);

			if (ray.direction[axis] < 0)
				return IntersectRay(farNode, ray, hitInfo, maxDistance, tMin, tMax);

			return IntersectRay(nearNode, ray, hitInfo, maxDistance, tMin, tMax);
		}
	}

}


template<typename T>
T* KDTree::Allocate(uint32_t count) 
{
	return (T*) _aligned_malloc(sizeof(T) * count, sizeof(KDTreeNode));
}

template<typename T>
void KDTree::Free(T* instance)
{
	instance->~T();
	_aligned_free(instance);
}

