#include "awesomerenderer.h"
#include "octree.h"

using namespace AwesomeRenderer;

Octree::Octree(const Vector3& center, const Vector3& halfSize) : center(center), halfSize(halfSize)
{
	
	for (int i = 0; i < 8; ++i)
		children[i] = NULL;

}

Octree::~Octree()
{
	for (int i = 0; i < 8; ++i)
		delete children[i];
}

// Determine which octant of the tree would contain 'point'
int Octree::octantForPoint(const Vector3& point) const
{
	int oct = 0;

	if (point[0] >= center[0]) oct |= 4;
	if (point[1] >= center[1]) oct |= 2;
	if (point[2] >= center[2]) oct |= 1;

	return oct;
}

bool Octree::isLeafNode() const 
{
	return children[0] == NULL;
}

