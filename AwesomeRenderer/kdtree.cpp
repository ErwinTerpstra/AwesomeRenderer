#include "awesomerenderer.h"

using namespace AwesomeRenderer;

const int KDTree::MAX_NODES_PER_LEAF = 1;
const int KDTree::MAX_DEPTH = 32;

KDTree::KDTree(KDTree* parent) : parent(parent), upperNode(NULL), lowerNode(NULL), objects()
{
	axis = (parent->axis + 1) % 3;
}

KDTree::~KDTree()
{
	if (upperNode)
		delete upperNode;

	if (lowerNode)
		delete lowerNode;

	objects.clear();
}

void KDTree::Optimize(int depth)
{
	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (objects.size() <= MAX_NODES_PER_LEAF)
		return;
	
	Split();
	
	// Create a plane that represents the split we made
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;
	Plane splitPlane(splitPoint, normal);

	// Relocate all objects in this leaf to the child nodes they intersect
	std::vector<const Object*>::const_iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		const Object* object = *it;
		const Shape& shape = object->GetBounds();

		// Determine which side of the plane this object is
		int side = shape.SideOfPlane(splitPlane);

		if (side >= 0)
			upperNode->objects.push_back(object);

		if (side <= 0)
			lowerNode->objects.push_back(object);
	}

	// Clear object list since this is no longer a leaf
	objects.clear();

	// If we are within the maximum tree depth, try to optimize child nodes
	if (depth < MAX_DEPTH)
	{
		upperNode->Optimize(depth + 1);
		lowerNode->Optimize(depth + 1);
	}
}

void KDTree::Split()
{


}