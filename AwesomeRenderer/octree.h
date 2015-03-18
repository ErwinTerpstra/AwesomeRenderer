#ifndef _OCTREE_H_
#define _OCTREE_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	
	class Octree
	{
	public:
		Vector3 center, halfSize;

		Octree* children[8];

	public:
		Octree(const Vector3& center, const Vector3& halfSize);
		~Octree();
		
		int octantForPoint(const Vector3& point) const;

		bool isLeafNode() const;



	};

}

#endif