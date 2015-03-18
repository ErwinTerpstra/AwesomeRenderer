#ifndef _KD_TREE_H_
#define _KD_TREE_H_

#include "awesomerenderer.h"
#include "aabb.h"

namespace AwesomeRenderer
{
	class Object;

	class KDTree
	{

	public:
		static const int MAX_NODES_PER_LEAF;
		static const int MAX_DEPTH;
		static const float TRAVERSAL_COST;
		static const float INTERSECTION_COST;
		static const float POSITION_EPSILON;

		KDTree *upperNode, *lowerNode;

		std::vector<const Object*> objects;

	private:
		KDTree *parent;

		int axis;

		float splitPoint;

	public:

		KDTree(KDTree* parent = NULL);
		~KDTree();

		void Optimize(const AABB& bounds, int depth = 0);

		bool IsLeaf() const { return upperNode == NULL && lowerNode == NULL; }

		float SplitPoint() const { return splitPoint; }
		int Axis() const { return axis; }
		KDTree* Parent() const { return parent; }

	private:
		void Split(const AABB& bounds);
		
		void CalculateBounds(const AABB& bounds, float splitPoint, AABB& upper, AABB& lower);

	};
}

#endif