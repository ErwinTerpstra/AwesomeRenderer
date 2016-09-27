#include "stdafx.h"
#include "awesomerenderer.h"
#include "kdtree.h"
#include "treeelement.h"

using namespace AwesomeRenderer;

const int KDTree::MAX_NODES_PER_LEAF = 8;
const int KDTree::MAX_DEPTH = 20;

const float KDTree::TRAVERSAL_COST = 1.0f;
const float KDTree::INTERSECTION_COST = 1.0f;
const float KDTree::POSITION_EPSILON = FLT_EPSILON * 2.0f;

KDTree::KDTree(KDTree* parent) : parent(parent), upperNode(NULL), lowerNode(NULL), elements(), bounds(Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX), Vector3(FLT_MAX, FLT_MAX, FLT_MAX))
{
	if (parent)
		axis = (parent->axis + 1) % 3;
	else
		axis = 0;
}

KDTree::~KDTree()
{
	if (upperNode)
		delete upperNode;

	if (lowerNode)
		delete lowerNode;

	elements.clear();
}

void KDTree::Optimize(const AABB& bounds, int depth)
{
	this->bounds = bounds;

	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (elements.size() <= MAX_NODES_PER_LEAF)
		return;

	if (depth >= MAX_DEPTH)
		return;

	SplitFast(bounds);

	upperNode = new KDTree(this);
	lowerNode = new KDTree(this);

	// Create a plane that represents the split we made
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;
	Plane splitPlane(splitPoint, normal);

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

void KDTree::Split(const AABB& bounds)
{
	// Initialize a potentional position list with two positions per object
	std::vector<float> splitPositions(elements.size() * 2);
	std::vector<float>::iterator positionIterator;

	// Iterate through all objects in this leaf
	for (auto it = elements.begin(); it != elements.end(); ++it)
	{
		// Calculate axis aligned boundaries for this shape
		AABB objectBounds;
		(*it)->GetPrimitive().CalculateBounds(objectBounds);

		// Save the minimum and maximum position for the bounds
		splitPositions.push_back(objectBounds.Min()[axis] - POSITION_EPSILON);
		splitPositions.push_back(objectBounds.Max()[axis] + POSITION_EPSILON);
	}

	// Sort split positions on the axis
	std::sort(splitPositions.begin(), splitPositions.end());

	float lowestCost = FLT_MAX;
	float lowestSplitPoint;

	// Normal vector for the split plane we create
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;

	// Copy the object list to keep count of which objects are potentionally above the current split plane
	std::vector<const TreeElement*> objectsAbove(elements.begin(), elements.end());

	// Iterate through the selected split positions
	for (positionIterator = splitPositions.begin(); positionIterator != splitPositions.end(); ++positionIterator)
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
		float cost = TRAVERSAL_COST + INTERSECTION_COST * (upperArea * upperObjectCount + lowerArea * lowerObjectCount);

		// Do we have a new lowest cost split point?
		if (cost < lowestCost)
		{
			lowestCost = cost;
			lowestSplitPoint = point;
		}
	}

	splitPoint = lowestSplitPoint;
}

void KDTree::SplitFast(const AABB& bounds)
{
	float min = bounds.Min()[axis];
	float max = bounds.Max()[axis];

	splitPoint = (min + max) / 2.0f;
}


void KDTree::CalculateBounds(const AABB& bounds, float splitPoint, AABB& upper, AABB& lower)
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

bool KDTree::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	RaycastHit boundsHitInfo;
	if (!bounds.IntersectRay(ray, boundsHitInfo))
		return false;

	float closestDistance = FLT_MAX;
	bool hit = false;

	// The current node is a leaf node, this means we can check its contents
	if (IsLeaf())
	{
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
	}
	else
	{
		RaycastHit upperHit;
		if (upperNode->IntersectRay(ray, upperHit) && upperHit.distance < closestDistance)
		{
			closestDistance = upperHit.distance;
			hitInfo = upperHit;
			hit = true;
		}

		RaycastHit lowerHit;
		if (lowerNode->IntersectRay(ray, lowerHit) && lowerHit.distance < closestDistance)
		{
			closestDistance = lowerHit.distance;
			hitInfo = lowerHit;
			hit = true;
		}
	}

	return hit;
}