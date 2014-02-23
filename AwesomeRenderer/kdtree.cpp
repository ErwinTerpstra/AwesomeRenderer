#include "awesomerenderer.h"

using namespace AwesomeRenderer;

const int KDTree::MAX_NODES_PER_LEAF = 5;
const int KDTree::MAX_DEPTH = 16;

const float KDTree::TRAVERSAL_COST = 1.0f;
const float KDTree::INTERSECTION_COST = 1.0f;
const float KDTree::POSITION_EPSILON = FLT_EPSILON * 2.0f;

KDTree::KDTree(KDTree* parent) : parent(parent), upperNode(NULL), lowerNode(NULL), objects()
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

	objects.clear();
}

void KDTree::Optimize(const AABB& bounds, int depth)
{
	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (objects.size() <= MAX_NODES_PER_LEAF)
		return;
	
	Split(bounds);

	// Create a plane that represents the split we made
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;
	Plane splitPlane(splitPoint, normal);

	// Relocate all objects in this leaf to the child nodes they intersect
	std::vector<const Object*>::const_iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		const Object* object = *it;
		const Shape& shape = object->GetShape();

		// Determine which side of the plane this object is
		int side = shape.SideOfPlane(splitPlane);

		if (side >= 0)
			upperNode->objects.push_back(object);

		if (side <= 0)
			lowerNode->objects.push_back(object);
	}

	//printf("Split @ depth %d (point %.5f)\n", depth, splitPoint);
	//printf("Object upper: %d; Object lower: %d\n", upperNode->objects.size(), lowerNode->objects.size());

	// Clear object list since this is no longer a leaf
	objects.clear();

	// If we are within the maximum tree depth, try to optimize child nodes
	if (depth < MAX_DEPTH)
	{
		AABB upperBounds, lowerBounds;

		CalculateBounds(bounds, splitPoint, upperBounds, lowerBounds);
		upperNode->Optimize(upperBounds, depth + 1);
		lowerNode->Optimize(lowerBounds, depth + 1);
	}
}

void KDTree::Split(const AABB& bounds)
{
	// Initialize a potentional position list with two positions per object
	std::vector<float> splitPositions(objects.size() * 2);
	std::vector<float>::iterator positionIterator;
	std::vector<const Object*>::iterator objectIterator;

	// Iterate through all objects in this leaf
	for (objectIterator = objects.begin(); objectIterator != objects.end(); ++objectIterator)
	{
		// Calculate axis aligned boundaries for this shape
		AABB objectBounds;
		(*objectIterator)->GetShape().CalculateBounds(objectBounds);

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
	std::vector<const Object*> objectsAbove(objects.begin(), objects.end());

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
		int lowerObjectCount = objects.size() - objectsAbove.size(); // Start with the number of objects we already know that are lower than this

		// Iterate through all objects to count how many objects fall on each side of the split plane
		for (objectIterator = objectsAbove.begin(); objectIterator != objectsAbove.end(); )
		{
			const Shape& shape = (*objectIterator)->GetShape();

			// Determine which side of the plane this object is
			int side = shape.SideOfPlane(splitPlane);

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

	upperNode = new KDTree(this);
	lowerNode = new KDTree(this);
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