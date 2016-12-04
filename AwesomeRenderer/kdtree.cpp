#include "stdafx.h"
#include "awesomerenderer.h"
#include "kdtree.h"
#include "kdtreenode.h"
#include "treeelement.h"

using namespace AwesomeRenderer;
const float KDTree::TRAVERSAL_COST = 1.0f;
const float KDTree::INTERSECTION_COST = 1.0f;
const float KDTree::POSITION_EPSILON = 1e-7f;
const float KDTree::MAX_OVERLAPPING_ELEMENTS = 0.5f;

KDTree::KDTree(uint32_t maxElementsPerLeaf, uint32_t maxDepth) :
	maxElementsPerLeaf(maxElementsPerLeaf), maxDepth(maxDepth)
{
	rootNode = new KDTreeNode(this);
}

KDTree::~KDTree()
{
	delete rootNode;
}

void KDTree::Optimize(const AABB& bounds)
{
	this->bounds = bounds;

	rootNode->Optimize(bounds);
}

void KDTree::Analyze() const
{
	uint32_t leaves = 0;
	uint32_t emptyLeaves = 0;
	uint32_t crampedLeaves = 0;
	float leafFillPercentage = 0.0f;

	std::vector<const KDTreeNode*> nodesLeft;

	nodesLeft.push_back(rootNode);

	while (!nodesLeft.empty())
	{
		const KDTreeNode* node = nodesLeft.back();
		nodesLeft.pop_back();

		if (node->IsLeaf())
		{
			++leaves;

			uint32_t elements = node->elements.size();
			if (elements > 0)
				leafFillPercentage += elements / (float)maxElementsPerLeaf;
			else
				++emptyLeaves;

			if (elements > maxElementsPerLeaf)
				++crampedLeaves;
		}
		else
		{
			nodesLeft.push_back(node->upperNode);
			nodesLeft.push_back(node->lowerNode);
		}
	}

	leafFillPercentage /= (leaves - emptyLeaves);

	printf("[KDTree]: Optimized with %u leaves, %u empty, %u cramped. Avg. fill percentage: %.0f%%\n",
		leaves, emptyLeaves, crampedLeaves, (leafFillPercentage * 100));
}


bool KDTree::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	float tMax, tMin;
	if (!bounds.IntersectRay(ray, tMin, tMax))
		return false;

	return rootNode->IntersectRay(ray, hitInfo, tMin, tMax);
}