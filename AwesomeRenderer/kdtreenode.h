#ifndef _KD_TREE_NODE_H_
#define _KD_TREE_NODE_H_

#include "awesomerenderer.h"
#include "aabb.h"

namespace AwesomeRenderer
{
	class TreeElement;
	class KDTree;

	class KDTreeNode
	{

	public:

		KDTreeNode *upperNode, *lowerNode;

		std::vector<const TreeElement*> elements;

	private:
		KDTree* tree;
		KDTreeNode *parent;

		int axis;

		float splitPoint;

	public:
		KDTreeNode(KDTree* tree, KDTreeNode* parent = NULL);

		~KDTreeNode();

		void Optimize(const AABB& bounds, int depth = 0);

		bool IsLeaf() const { return upperNode == NULL && lowerNode == NULL; }

		float SplitPoint() const { return splitPoint; }
		int Axis() const { return axis; }
		KDTreeNode* Parent() const { return parent; }

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const;

	private:
		void SplitSAH(const AABB& bounds);
		void SplitMode(const AABB& bounds);
		void SplitFast(const AABB& bounds);

		void CalculateBounds(const AABB& bounds, float splitPoint, AABB& upper, AABB& lower);
	};
}

#endif