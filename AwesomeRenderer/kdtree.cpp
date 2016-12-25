#include "stdafx.h"
#include "awesomerenderer.h"
#include "kdtree.h"
#include "kdtreenode.h"
#include "treeelement.h"

#define NEW(type, args) new (Allocate<type>()) type(args)

using namespace AwesomeRenderer;
const float KDTree::TRAVERSAL_COST = 0.2f;
const float KDTree::INTERSECTION_COST = 1.0f;
const float KDTree::POSITION_EPSILON = 1e-5f;

KDTree::KDTree(uint32_t maxDepth) : maxDepth(maxDepth)
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
	uint32_t maxDepth = 0;
	uint32_t largestLeafSize = 0;
	uint32_t totalElements = 0;

	struct ScheduledNode
	{
		const KDTreeNode* ptr;
		uint32_t depth;
	};


	std::vector<ScheduledNode> nodesLeft;

	ScheduledNode node;
	node.ptr = rootNode;
	node.depth = 0;

	nodesLeft.push_back(node);

	while (!nodesLeft.empty())
	{
		node = nodesLeft.back();
		nodesLeft.pop_back();

		if (node.ptr->IsLeaf())
		{
			++leaves;

			uint32_t elements = node.ptr->GetElements().size();
			if (elements == 0)
				++emptyLeaves;

			totalElements += elements;
			largestLeafSize = std::max(largestLeafSize, elements);

			maxDepth = std::max(maxDepth, node.depth);
		}
		else
		{
			++node.depth;

			const KDTreeNode* oldNode = node.ptr;
			node.ptr = oldNode->GetUpperNode();
			nodesLeft.push_back(node);

			node.ptr = oldNode->GetLowerNode();
			nodesLeft.push_back(node);
		}
	}

	printf("[KDTree]: Optimized with %u leaves, %u empty; Largest leaf: %u/%u; Max depth: %u\n",
		leaves, emptyLeaves, largestLeafSize, totalElements, maxDepth);
}

void KDTree::Optimize(KDTreeNode* node, const AABB& bounds, int depth)
{
	assert(node->IsLeaf());

	KDTreeNode::ElementList& elements = node->GetElements();

	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (depth >= maxDepth)
		return;

	// If the splitting algorithm can't find a split point which whill improve the tree, leave it as is
	if (!SplitSAH(node, bounds))
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
	float dontSplitCost = KDTree::INTERSECTION_COST * node->GetElements().size();

	int bestAxis = -1;
	float lowestCost = FLT_MAX;
	float bestSplitPosition;

	const Vector3& min = bounds.Min();
	const Vector3& max = bounds.Max();

	/*
	// Find the longest axis and use that as split axis
	float dx = max[0] - min[0];
	float dy = max[1] - min[1];
	float dz = max[2] - min[2];

	if (dx > dz)
	{
		if (dx > dy)
			bestAxis = 0;
		else
			bestAxis = 1;
	}
	else
	{
		if (dz > dy)
			bestAxis = 2;
		else
			bestAxis = 1;
	}
	*/

	bestAxis = node->GetAxis();
	SplitSAH(node, bestAxis, bounds, bestSplitPosition, lowestCost);

	/*/
	// Find the best axis to split along by calculating the split cost for each axis
	for (int axis = 0; axis < 3; ++axis)
	{
		float splitPosition, cost;
		SplitSAH(node, axis, bounds, splitPosition, cost);

		if (cost < lowestCost)
		{
			lowestCost = cost;
			bestAxis = axis;
			bestSplitPosition = splitPosition;
		}
	}

	//*/

	if (lowestCost <= dontSplitCost)
	{
		node->SetAxis(bestAxis);
		node->SetSplitPoint(bestSplitPosition);

		return true;
	}
	else
		return false;
}

void KDTree::SplitSAH(KDTreeNode* node, int axis, const AABB& bounds, float& bestSplitPosition, float& lowestCost)
{
	KDTreeNode::ElementList& elements = node->GetElements();

	const Vector3& max = bounds.Max();
	const Vector3& min = bounds.Min();

	// Initialize a potentional position list with two positions per object
	std::vector<SplitPosition> splitPositions;
	splitPositions.reserve(elements.size() * 2);
	
	// Iterate through all objects in this leaf
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		// Calculate axis aligned boundaries for this shape
		AABB objectBounds;
		(*it)->GetPrimitive().CalculateBounds(objectBounds);

		// Save the minimum and maximum position for the bounds
		SplitPosition splitPosition;

		float oMin = objectBounds.Min()[axis] - KDTree::POSITION_EPSILON;
		float oMax = objectBounds.Max()[axis] + KDTree::POSITION_EPSILON;

		// If the primitive intersects the outer bounds, skip it
		if (oMin <= min[axis] || oMax >= max[axis])
			continue;

		// If the primitive has zero length on this axis, skip it
		if (fabs(oMin - oMax) < KDTree::POSITION_EPSILON)
			continue;

		splitPosition.position = oMin;
		splitPosition.event = SplitPosition::PRIMITIVE_START;
		splitPositions.push_back(splitPosition);
		
		splitPosition.position = oMax;
		splitPosition.event = SplitPosition::PRIMITIVE_END;
		splitPositions.push_back(splitPosition);
	}

	// Sort split positions on the axis
	std::sort(splitPositions.begin(), splitPositions.end(), SortSplitPosition);

	// Calculate the length of this node
	float rootLength = max[axis] - min[axis];

	// Calculate the surface of this node
	int upperAxis = (axis + 1) % 3;
	int lowerAxis = axis == 0 ? 2 : axis - 1;
	float width = max[upperAxis] - min[upperAxis];
	float height = max[lowerAxis] - min[lowerAxis];
	float depth = max[axis] - min[axis];
	float rootArea = bounds.CalculateSurfaceArea();

	int lowerObjectCount = 0;
	int upperObjectCount = elements.size();

	bool addToLower = false;

	lowestCost = FLT_MAX;

	// Iterate through the selected split positions
	for (uint32_t pointIdx = 0, pointCount = splitPositions.size(); pointIdx < pointCount; ++pointIdx)
	{
		const SplitPosition& splitPosition = splitPositions[pointIdx];

		if (addToLower)
			++lowerObjectCount;

		if (splitPosition.event == SplitPosition::PRIMITIVE_START)
		{
			addToLower = true;
		}
		else if (splitPosition.event == SplitPosition::PRIMITIVE_END)
		{
			--upperObjectCount;

			addToLower = false;
		}
		
		// Calculate length for upper and lower nodes
		float upperLength = max[axis] - splitPosition.position;
		float lowerLength = splitPosition.position - min[axis];

		assert(lowerObjectCount >= 0 && upperObjectCount >= 0);
		assert(lowerObjectCount + upperObjectCount >= elements.size());

		// Calculate final cost for splitting at this point
		
		/*
		float cost = KDTree::TRAVERSAL_COST + KDTree::INTERSECTION_COST * ((upperLength / rootLength) * upperObjectCount +
																			(lowerLength / rootLength) * lowerObjectCount);
		/*/
		float upperArea = 2 * (width * height + height * upperLength + width * upperLength);
		float lowerArea = 2 * (width * height + height * lowerLength + width * lowerLength);
		float cost = KDTree::TRAVERSAL_COST + KDTree::INTERSECTION_COST * ((upperArea / rootArea) * upperObjectCount +
																			(lowerArea / rootArea) * lowerObjectCount);
		//*/

		assert(fabs(rootLength - (upperLength + lowerLength)) < 1e-3f);

		// Do we have a new lowest cost split point?
		if (cost < lowestCost)
		{
			lowestCost = cost;
			bestSplitPosition = splitPosition.position;
		}
	}
	
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
	
	if (elements.size() < 10)
		return false;

	const Vector3& min = bounds.Min();
	const Vector3& max = bounds.Max();

	// Find the longest axis and use that as split axis
	float dx = max[0] - min[0];
	float dy = max[1] - min[1];
	float dz = max[2] - min[2];

	int axis;
	if (dx > dz)
	{
		if (dx > dy)
			axis = 0;
		else
			axis = 1;
	}
	else
	{
		if (dz > dy)
			axis = 2;
		else
			axis = 1;
	}

	// The split point is halfway this axis
	float splitPoint = (min[axis] + max[axis]) / 2.0f;
	node->SetAxis(axis);
	node->SetSplitPoint(splitPoint);
	
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

bool KDTree::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	float tMax, tMin;
	if (!bounds.IntersectRay(ray, tMin, tMax))
		return false;

	if (tMin > maxDistance)
		return false;

	//return IntersectRayRec(rootNode, ray, hitInfo, std::max(0.0f, tMin), std::min(maxDistance, tMax));
	return IntersectRaySec(ray, hitInfo, std::max(0.0f, tMin), std::min(maxDistance, tMax));
}

bool KDTree::IntersectRayRec(KDTreeNode* node, const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	// The current node is a leaf node, this means we can check its contents
	if (node->IsLeaf())
	{
		float closestDistance = tMax;
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
			return IntersectRayRec(nearNode, ray, hitInfo, tMin, tMax);

		if (tSplit >= tMin)
		{
			if (tSplit > 0)
			{
				// We should only return this intersection if the intersection point is inside the node...
				if (IntersectRayRec(nearNode, ray, hitInfo, tMin, tSplit))
					return true;

				return IntersectRayRec(farNode, ray, hitInfo, tSplit, tMax);
			}

			return IntersectRayRec(nearNode, ray, hitInfo, tSplit, tMax);
		}
		else // tSplit < tMin
		{
			if (tSplit > 0)
				return IntersectRayRec(farNode, ray, hitInfo, tMin, tMax);

			if (tSplit < 0)
				return IntersectRayRec(nearNode, ray, hitInfo, tMin, tMax);

			if (ray.direction[axis] < 0)
				return IntersectRayRec(farNode, ray, hitInfo, tMin, tMax);

			return IntersectRayRec(nearNode, ray, hitInfo, tMin, tMax);
		}
	}

}

bool KDTree::IntersectRaySec(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	TraversalStack stack;
	stack.Push(rootNode, tMin, tMax);

	while (!stack.IsEmpty())
	{
		const StackNode& stackNode = stack.Pop();
		const KDTreeNode* node = stackNode.node;

		tMin = stackNode.tMin;
		tMax = stackNode.tMax;

		while (!node->IsLeaf())
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

			if (tSplit >= tMax || tSplit < 0)
				node = nearNode;
			else if (tSplit <= tMin)
				node = farNode;
			else
			{
				stack.Push(farNode, tSplit, tMax);

				node = nearNode;
				tMax = tSplit;
			}
		}

		// The current node is a leaf node, this means we can check its contents
		float closestDistance = tMax;
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

		if (hit)
			return true;
	}

	return false;
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

