#include "awesomerenderer.h"

using namespace AwesomeRenderer;

const int KDTree::MAX_NODES_PER_LEAF = 1;
const int KDTree::MAX_DEPTH = 32;

KDTree::KDTree(KDTree* parent) : parent(parent), leftNode(NULL), rightNode(NULL), objects()
{

}

KDTree::~KDTree()
{
	if (leftNode)
		delete leftNode;

	if (rightNode)
		delete rightNode;

	objects.clear();
}

void KDTree::Optimize(int depth)
{
	// If we are within the maximum number of objects per leaf we can leave this node as is
	if (objects.size() <= MAX_NODES_PER_LEAF)
		return;
	
	Split();

	// Relocate all objects in this leaf to the child nodes they intersect
	std::vector<Object*>::iterator it;

	for (it = objects.begin(); it != objects.end(); ++it)
	{
		const Object* object = *it;
		const Shape& shape = object->GetBounds();


	}

	// Clear object list since this is no longer a leaf
	objects.clear();

	// If we are within the maximum tree depth, try to optimize child nodes
	if (depth < MAX_DEPTH)
	{
		leftNode->Optimize(depth + 1);
		rightNode->Optimize(depth + 1);
	}
}

void KDTree::Split()
{
	
}