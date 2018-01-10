
#include "awesomerenderer.h"
#include "kdtree.h"
#include "kdtreenode.h"

using namespace AwesomeRenderer;

template <typename ElementType>
const float KDTree<ElementType>::TRAVERSAL_COST = 1.0f;

template <typename ElementType>
const float KDTree<ElementType>::INTERSECTION_COST = 80.0f;

template <typename ElementType>
const float KDTree<ElementType>::EMPTY_BONUS = 0.0f;

template <typename ElementType>
const float KDTree<ElementType>::POSITION_EPSILON = 1e-5f;


template <typename ElementType>
KDTree<ElementType>::KDTree(uint32_t maxDepth) : maxDepth(maxDepth), elements(), nodes(NULL), lastNode(0), availableNodes(0), elementBuffer(NULL)
{

}

template <typename ElementType>
KDTree<ElementType>::~KDTree()
{
	if (nodes != NULL)
		FreeAligned(nodes);

	if (elementBuffer != NULL)
		FreeAligned(elementBuffer);
}

template <typename ElementType>
void KDTree<ElementType>::Optimize(const AABB& bounds)
{
	this->bounds = bounds;
	
	// Create the index buffer which holds a list of element indices in all nodes
	elementBufferSize = elements.size() * maxDepth * 2;
	elementBufferOffset = 0;
	elementBuffer = AllocateAligned<ElementType*>(4, elementBufferSize);
	
	// Create the root node and recursively build the tree
	CreateNode(0, bounds, elements);
}

template <typename ElementType>
void KDTree<ElementType>::Analyze() const
{
	uint32_t leaves = 0;
	uint32_t emptyLeaves = 0;
	uint32_t maxDepth = 0;
	uint32_t largestLeafSize = 0;
	uint32_t totalElements = 0;

	struct ScheduledNode
	{
		const KDTreeNode<ElementType>* ptr;
		uint32_t depth;
	};


	std::vector<ScheduledNode> nodesLeft;

	ScheduledNode node;
	node.ptr = nodes;
	node.depth = 0;

	nodesLeft.push_back(node);

	while (!nodesLeft.empty())
	{
		node = nodesLeft.back();
		nodesLeft.pop_back();

		if (node.ptr->IsLeaf())
		{
			++leaves;

			uint32_t elements = node.ptr->GetElementCount();
			if (elements == 0)
				++emptyLeaves;

			totalElements += elements;
			largestLeafSize = std::max(largestLeafSize, elements);

			maxDepth = std::max(maxDepth, node.depth);
		}
		else
		{
			++node.depth;

			const KDTreeNode<ElementType>* oldNode = node.ptr;
			node.ptr = nodes + oldNode->GetUpperNode();
			nodesLeft.push_back(node);

			node.ptr = nodes + oldNode->GetUpperNode() + 1;
			nodesLeft.push_back(node);
		}
	}

	printf("[KDTree]: Optimized with %u leaves, %u empty; Largest leaf: %u/%u; Max depth: %u\n",
		leaves, emptyLeaves, largestLeafSize, totalElements, maxDepth);
}

template <typename ElementType>
void KDTree<ElementType>::CreateNode(uint32_t nodeIdx, const AABB& bounds, const std::vector<ElementType*>& elements, int depth)
{
	if (nodeIdx >= availableNodes)
	{
		uint32_t desiredNodes = std::max(2 * availableNodes, 512U);
		KDTreeNode<ElementType>* newNodes = AllocateAligned<KDTreeNode<ElementType>>(8, desiredNodes);
		if (availableNodes > 0)
		{
			memcpy(newNodes, nodes, availableNodes * sizeof(KDTreeNode<ElementType>));
			FreeAligned(nodes);
		}

		nodes = newNodes;
		availableNodes = desiredNodes;
	}
	
	KDTreeNode<ElementType>* node = nodes + nodeIdx;

	if (nodeIdx > lastNode)
		lastNode = nodeIdx;

	// If we are at the maximum tree depth, this will always be a leaf node
	if (depth >= maxDepth)
	{
		InitialiseLeaf(node, elements);
		return;
	}

	// TODO: check if this is necessary
	float volume = bounds.Width() * bounds.Height() * bounds.Depth();
	if (volume < 1e-3f)
	{
		InitialiseLeaf(node, elements);
		return;
	}
		
	// If the splitting algorithm can't find a split point which whill improve the tree, create a leaf
	int axis = depth % 3;
	float splitPoint, splitCost;
	//if (!SplitFast(elements, bounds, axis, splitPoint))
	if (!SplitSAH(axis, elements, bounds, splitPoint, splitCost))
	//if (!SplitSAH(elements, bounds, axis, splitPoint)) // Choose best axis to split this node
	{
		InitialiseLeaf(node, elements);
		return;
	}

	std::vector<ElementType*> upperElements;
	std::vector<ElementType*> lowerElements;

	// Relocate all objects in this leaf to the child nodes they intersect
	uint32_t elementCount = elements.size();
	for (uint32_t elementIdx = 0; elementIdx < elementCount; ++elementIdx)
	{
		const Primitive& primitive = elements[elementIdx]->GetPrimitive();

		// Determine which side of the plane this object is
		int side = primitive.SideOfAAPlane(axis, splitPoint);

		if (side >= 0)
			upperElements.push_back(elements[elementIdx]);

		if (side <= 0)
			lowerElements.push_back(elements[elementIdx]);
	}
	
	uint32_t upperNodeIdx = lastNode + 1;
	lastNode += 2;
	node->InitialiseNonLeaf(splitPoint, axis, upperNodeIdx);

	// Try to optimize child nodes
	AABB upperBounds, lowerBounds;
	CalculateBounds(bounds, axis, splitPoint, upperBounds, lowerBounds);
	
	CreateNode(upperNodeIdx, upperBounds, upperElements, depth + 1);
	CreateNode(upperNodeIdx + 1, lowerBounds, lowerElements, depth + 1);
}

template <typename ElementType>
void KDTree<ElementType>::InitialiseLeaf(KDTreeNode<ElementType>* node, const std::vector<ElementType*>& elements)
{
	uint32_t elementCount = elements.size();

	assert((elementBufferOffset + elementCount) < elementBufferSize);

	// Reserve space in the element index bufer for all indices
	ElementType** nodeElements = elementBuffer + elementBufferOffset;
	elementBufferOffset += elementCount;

	// Copy indices to the buffer for the node
	for (uint32_t elementIdx = 0; elementIdx < elementCount; ++elementIdx)
		nodeElements[elementIdx] = elements[elementIdx];

	node->InitialiseLeaf(nodeElements, elementCount);
}

template <typename ElementType>
bool KDTree<ElementType>::SplitSAH(const std::vector<ElementType*>& elements, const AABB& bounds, int& bestAxis, float& bestSplitPosition)
{
	bool willSplit = false;
	float lowestCost = FLT_MAX;

	const Vector3& min = bounds.Min();
	const Vector3& max = bounds.Max();

	// Find the best axis to split along by calculating the split cost for each axis
	for (int axis = 0; axis < 3; ++axis)
	{
		float splitPosition, cost;
		if (SplitSAH(axis, elements, bounds, splitPosition, cost))
		{
			willSplit = true;

			if (cost < lowestCost)
			{
				lowestCost = cost;
				bestAxis = axis;
				bestSplitPosition = splitPosition;
			}
		}
	}
	
	return willSplit;
}


template <typename ElementType>
bool KDTree<ElementType>::SplitSAH(int axis, const std::vector<ElementType*>& elements, const AABB& bounds, float& bestSplitPosition, float& lowestCost)
{
	const Vector3& max = bounds.Max();
	const Vector3& min = bounds.Min();

	uint32_t elementCount = elements.size();
	float dontSplitCost = KDTree<ElementType>::INTERSECTION_COST * elementCount;

	// Initialize a potentional position list with two positions per object
	splitPositions.clear();
	splitPositions.reserve(elementCount * 2);
	
	// Iterate through all objects in this leaf
	for (uint32_t elementIdx = 0; elementIdx < elementCount; ++elementIdx)
	{
		// Calculate axis aligned boundaries for this shape
		AABB objectBounds;
		elements[elementIdx]->GetPrimitive().CalculateBounds(objectBounds);

		// Save the minimum and maximum position for the bounds
		SplitPosition splitPosition;

		float oMin = objectBounds.Min()[axis] - KDTree<ElementType>::POSITION_EPSILON;
		float oMax = objectBounds.Max()[axis] + KDTree<ElementType>::POSITION_EPSILON;

		// If the primitive intersects the outer bounds, skip it
		if (oMin <= min[axis] || oMax >= max[axis])
			continue;

		// If the primitive has zero length on this axis, skip it
		if (fabs(oMin - oMax) < KDTree<ElementType>::POSITION_EPSILON)
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
	int lowerAxis = (axis + 2) % 3; 
	float width = max[upperAxis] - min[upperAxis];
	float height = max[lowerAxis] - min[lowerAxis];
	float depth = max[axis] - min[axis];
	float rootArea = bounds.Area();

	int lowerObjectCount = 0;
	int upperObjectCount = elementCount;

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
		assert(lowerObjectCount + upperObjectCount >= elementCount);

		// Calculate final cost for splitting at this point
		float upperArea = 2 * (width * height + height * upperLength + width * upperLength);
		float lowerArea = 2 * (width * height + height * lowerLength + width * lowerLength);
		float emptyBonus = (lowerObjectCount == 0 || upperObjectCount == 0) ? KDTree<ElementType>::EMPTY_BONUS : 0.0f;

		float cost = KDTree<ElementType>::TRAVERSAL_COST + KDTree<ElementType>::INTERSECTION_COST * (1.0f - emptyBonus) *
												((upperArea / rootArea) * upperObjectCount + (lowerArea / rootArea) * lowerObjectCount);

		assert(fabs(rootLength - (upperLength + lowerLength)) < 1e-3f);

		// Do we have a new lowest cost split point?
		if (cost < lowestCost)
		{
			lowestCost = cost;
			bestSplitPosition = splitPosition.position;
		}
	}

	return lowestCost < dontSplitCost;	
}

template <typename ElementType>
bool KDTree<ElementType>::SplitFast(const std::vector<ElementType*>& elements, const AABB& bounds, int& bestAxis, float& bestSplitPosition)
{
	if (elements.size() < 25)
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
	bestAxis = axis;
	bestSplitPosition = (min[axis] + max[axis]) / 2.0f;

	return true;
}

template <typename ElementType>
void KDTree<ElementType>::CalculateBounds(const AABB& bounds, int axis, float splitPoint, AABB& upper, AABB& lower)
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

template <typename ElementType>
bool KDTree<ElementType>::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	float tMax, tMin;
	if (!bounds.IntersectRay(ray, tMin, tMax))
		return false;

	if (tMin > maxDistance)
		return false;

	// Somehow, if we use std::min(maxDistance, tMax) as max distance, this sometimes misses intersection.
	// TODO: Test why this is.
	return IntersectRaySec(ray, hitInfo, std::max(0.0f, tMin), maxDistance);
	//return IntersectRayRec(&nodes[0], ray, hitInfo, std::max(0.0f, tMin), maxDistance);
}

template <typename ElementType>
bool KDTree<ElementType>::IntersectRayRec(KDTreeNode<ElementType>* node, const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	// The current node is a leaf node, this means we can check its contents
	if (node->IsLeaf())
	{
		float closestDistance = tMax;
		bool hit = false;

		ElementType** elements = node->GetElements();
		for (uint32_t elementIdx = 0, elementCount = node->GetElementCount(); elementIdx < elementCount; ++elementIdx)
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

		KDTreeNode<ElementType>* nearNode;
		KDTreeNode<ElementType>* farNode;

		if (ray.origin[axis] < splitPoint)
		{
			farNode = nodes + node->GetUpperNode();
			nearNode = farNode + 1;
		}
		else
		{
			nearNode = nodes + node->GetUpperNode();
			farNode = nearNode + 1;
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

template <typename ElementType>
bool KDTree<ElementType>::IntersectRaySec(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	TraversalStack stack;
	stack.Push(nodes, tMin, tMax);

	while (!stack.IsEmpty())
	{
		const StackNode& stackNode = stack.Pop();
		const KDTreeNode<ElementType>* node = stackNode.node;

		tMin = stackNode.tMin;
		tMax = stackNode.tMax;

		while (!node->IsLeaf())
		{
			int axis = node->GetAxis();
			float splitPoint = node->GetSplitPoint();
			
			const KDTreeNode<ElementType>* nearNode;
			const KDTreeNode<ElementType>* farNode;

			if (ray.origin[axis] < splitPoint)
			{
				farNode = &nodes[node->GetUpperNode()];
				nearNode = farNode + 1;
			}
			else
			{
				nearNode = &nodes[node->GetUpperNode()];
				farNode = nearNode + 1;
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

		uint32_t elementCount = node->GetElementCount();
		ElementType** elements = node->GetElements();
		for (uint32_t elementIdx = 0; elementIdx < elementCount; ++elementIdx)
		{
			const ElementType* element = elements[elementIdx];

			// Perform the ray-triangle intersection
			if (element->GetShape().IntersectRay(ray, hitInfo, closestDistance))
			{
				hitInfo.element = element;

				closestDistance = hitInfo.distance;
				hit = true;
			}
		}

		if (hit)
			return true;
	}

	return false;
}

#include "renderable.h"
template KDTree<Renderable>;

#include "meshtriangle.h"

template<>
bool KDTree<MeshTriangle>::IntersectRaySec(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	TraversalStack stack;
	stack.Push(nodes, tMin, tMax);

	while (!stack.IsEmpty())
	{
		const StackNode& stackNode = stack.Pop();
		const KDTreeNode<MeshTriangle>* node = stackNode.node;

		tMin = stackNode.tMin;
		tMax = stackNode.tMax;

		while (!node->IsLeaf())
		{
			int axis = node->GetAxis();
			float splitPoint = node->GetSplitPoint();

			const KDTreeNode<MeshTriangle>* nearNode;
			const KDTreeNode<MeshTriangle>* farNode;

			if (ray.origin[axis] < splitPoint)
			{
				farNode = &nodes[node->GetUpperNode()];
				nearNode = farNode + 1;
			}
			else
			{
				nearNode = &nodes[node->GetUpperNode()];
				farNode = nearNode + 1;
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
		hitInfo.distance = tMax;
		bool hit = false;

		uint32_t elementCount = node->GetElementCount();
		MeshTriangle** elements = node->GetElements();
		for (uint32_t elementIdx = 0; elementIdx < elementCount; ++elementIdx)
		{
			const MeshTriangle* element = elements[elementIdx];

			// Perform the ray-triangle intersection
			if (element->IntersectRay(ray, hitInfo, hitInfo.distance))
			{
				hitInfo.element = element;

				//closestDistance = hitInfo.distance;
				hit = true;
			}
		}

		if (hit)
			return true;
	}

	return false;
}

template KDTree<MeshTriangle>;