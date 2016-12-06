#ifndef _KD_TREE_H_
#define _KD_TREE_H_

#include "awesomerenderer.h"
#include "aabb.h"

namespace AwesomeRenderer
{
	class KDTreeNode;

	class KDTree
	{
		friend class KDTreeNode;

	public:
		static const float TRAVERSAL_COST;
		static const float INTERSECTION_COST;
		static const float POSITION_EPSILON;
		static const float PREVENT_SPLIT_MAX_OVERFLOW;
		static const float PREVENT_SPLIT_OVERLAPPING_ELEMENTS;

		KDTreeNode* rootNode;

	private:
		AABB bounds;

		uint32_t maxElementsPerLeaf;
		uint32_t maxDepth;
	public:
		KDTree(uint32_t maxElementsPerLeaf, uint32_t maxDepth);
		~KDTree();

		void Optimize(const AABB& bounds);
		void Analyze() const;

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance = FLT_MAX) const;

	};
}

#endif