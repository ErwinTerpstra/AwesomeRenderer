#include "stdafx.h"
#include "kdtreenode.h"
#include "kdtree.h"
#include "treeelement.h"

using namespace AwesomeRenderer;

KDTreeNode::KDTreeNode(KDTree* tree, KDTreeNode* parent) : 
	tree(tree), parent(parent), upperNode(NULL), lowerNode(NULL), elements()
{
	if (parent)
		axis = (parent->axis + 1) % 3;
	else
		axis = 0;
}

KDTreeNode::~KDTreeNode()
{
	if (upperNode)
		delete upperNode;

	if (lowerNode)
		delete lowerNode;

	elements.clear();
}

void KDTreeNode::Optimize(const AABB& bounds, int depth)
{
	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (elements.size() <= tree->maxElementsPerLeaf)
		return;

	if (depth >= tree->maxDepth)
		return;

	SplitFast(bounds);

	// Create a plane that represents the split we made
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;
	Plane splitPlane(splitPoint, normal);

	// Before performing the real split, check if it actually improves our tree
	uint32_t overlappingElements = 0;
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		const Primitive& primitive = (*it)->GetPrimitive();

		// Determine which side of the plane this object is
		int side = primitive.SideOfPlane(splitPlane);

		if (side == 0)
			++overlappingElements;
	}

	// Don't perform the split if elements end up in both nodes
	if (overlappingElements / (float)elements.size() >= KDTree::MAX_OVERLAPPING_ELEMENTS)
		return;

	upperNode = new KDTreeNode(tree, this);
	lowerNode = new KDTreeNode(tree, this);

	// Relocate all objects in this leaf to the child nodes they intersect
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		const Primitive& primitive = (*it)->GetPrimitive();

		// Determine which side of the plane this object is
		int side = primitive.SideOfPlane(splitPlane);

		if (side >= 0)
			upperNode->elements.push_back(*it);

		if (side <= 0)
			lowerNode->elements.push_back(*it);
	}

	//printf("Split @ depth %d (point %.5f)\n", depth, splitPoint);
	//printf("Object upper: %d; Object lower: %d\n", upperNode->objects.size(), lowerNode->objects.size());

	// Clear object list since this is no longer a leaf
	elements.clear();

	// Try to optimize child nodes
	AABB upperBounds, lowerBounds;

	CalculateBounds(bounds, splitPoint, upperBounds, lowerBounds);
	upperNode->Optimize(upperBounds, depth + 1);
	lowerNode->Optimize(lowerBounds, depth + 1);
}

void KDTreeNode::SplitSAH(const AABB& bounds)
{
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
	std::list<const TreeElement*> objectsAbove(elements.begin(), elements.end());

	// Iterate through the selected split positions
	for (auto positionIterator = splitPositions.begin(); positionIterator != splitPositions.end(); ++positionIterator)
	{
		float point = *positionIterator;

		// Calculate bounding area for the child nodes resulting from this split
		AABB upper, lower;
		CalculateBounds(bounds, point, upper, lower);

		// Create a plane that represents the split we made
		Plane splitPlane(point, normal);

		int upperObjectCount = 0;
		int lowerObjectCount = elements.size() - objectsAbove.size(); // Start with the number of objects we already know that are lower than this

																	  // Iterate through all objects to count how many objects fall on each side of the split plane
		for (auto objectIterator = objectsAbove.begin(); objectIterator != objectsAbove.end(); )
		{
			const Primitive& primitive = (*objectIterator)->GetPrimitive();

			// Determine which side of the plane this object is
			int side = primitive.SideOfPlane(splitPlane);

			if (side >= 0)
				++upperObjectCount;

			if (side <= 0)
				++lowerObjectCount;

			// Objects located below this split plane don't need to be checked the next iteration
			if (side < 0)
				objectIterator = objectsAbove.erase(objectIterator);
			else
				++objectIterator;
		}

		// Calculate area for upper and lower nodes
		float upperArea = 2 * upper.Width() * upper.Height() * upper.Depth();
		float lowerArea = 2 * lower.Width() * lower.Height() * lower.Depth();

		// Calculate final cost for splitting at this point
		float cost = KDTree::TRAVERSAL_COST + KDTree::INTERSECTION_COST * (upperArea * upperObjectCount + lowerArea * lowerObjectCount);

		// Do we have a new lowest cost split point?
		if (cost < lowestCost)
		{
			lowestCost = cost;
			lowestSplitPoint = point;
		}
	}

	splitPoint = lowestSplitPoint;
}

void KDTreeNode::SplitMode(const AABB& bounds)
{
	float median = 0.0f;

	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		AABB elementBounds;
		(*it)->GetPrimitive().CalculateBounds(elementBounds);

		float center = (elementBounds.Min()[axis] + elementBounds.Max()[axis]) / 2;
		median += center;
	}

	median /= elements.size();

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
}

void KDTreeNode::SplitFast(const AABB& bounds)
{
	float min = bounds.Min()[axis];
	float max = bounds.Max()[axis];

	splitPoint = (min + max) / 2.0f;
}

void KDTreeNode::CalculateBounds(const AABB& bounds, float splitPoint, AABB& upper, AABB& lower)
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

bool KDTreeNode::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const
{
	// The current node is a leaf node, this means we can check its contents
	if (IsLeaf())
	{
		float closestDistance = FLT_MAX;
		bool hit = false;

		for (auto it = elements.begin(); it != elements.end(); ++it)
		{
			const Shape& shape = (*it)->GetShape();

			// Perform the ray-triangle intersection
			RaycastHit shapeHitInfo;
			if (!shape.IntersectRay(ray, shapeHitInfo))
				continue;

			if (shapeHitInfo.distance > closestDistance)
				continue;

			closestDistance = shapeHitInfo.distance;

			hitInfo = shapeHitInfo;
			hitInfo.element = *it;

			hit = true;
		}

		return hit;
	}
	else
	{
		KDTreeNode* nearNode;
		KDTreeNode* farNode;

		if (ray.origin[axis] < splitPoint)
		{
			nearNode = lowerNode;
			farNode = upperNode;
		}
		else
		{
			nearNode = upperNode;
			farNode = lowerNode;
		}

		float tSplit = (splitPoint - ray.origin[axis]) / ray.direction[axis];

		if (tSplit > tMax)
			return nearNode->IntersectRay(ray, hitInfo, tMin, tMax);

		if (tSplit < tMin)
		{
			if (tSplit > 0)
				return farNode->IntersectRay(ray, hitInfo, tMin, tMax);

			if (tSplit < 0)
				return nearNode->IntersectRay(ray, hitInfo, tMin, tMax);

			if (ray.direction[axis] < 0)
				return farNode->IntersectRay(ray, hitInfo, tMin, tMax);

			return nearNode->IntersectRay(ray, hitInfo, tMin, tMax);
		}
		else
		{
			if (tSplit > 0)
			{
				if (nearNode->IntersectRay(ray, hitInfo, tMin, tMax))
				{
					// We should only return this intersection if the intersection point is inside the node...
					if (hitInfo.distance <= tSplit)
						return true;
				}

				return farNode->IntersectRay(ray, hitInfo, tMin, tMax);
			}

			return nearNode->IntersectRay(ray, hitInfo, tMin, tMax);
		}
	}

}