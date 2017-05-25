#ifndef _KD_TREE_H_
#define _KD_TREE_H_

#include "awesomerenderer.h"
#include "aabb.h"

namespace AwesomeRenderer
{
	template<typename ElementType>
	class KDTreeNode;

	template <typename ElementType>
	class KDTree
	{
	public:
		static const uint32_t MAX_DEPTH = 20;
		static const float TRAVERSAL_COST;
		static const float INTERSECTION_COST;
		static const float EMPTY_BONUS;
		static const float POSITION_EPSILON;

		std::vector<ElementType*> elements;

	private:
		struct SplitPosition
		{
			enum Event
			{
				PRIMITIVE_START,
				PRIMITIVE_END
			};

			float position;
			Event event;
		};

		struct StackNode
		{
			const KDTreeNode<ElementType>* node;
			float tMin, tMax;
		};

		class TraversalStack
		{
		public:
			StackNode stack[MAX_DEPTH];

			uint32_t count;
			
		public:
			TraversalStack() : count(0)
			{

			}

			void Push(const KDTreeNode<ElementType>* node, float tMin, float tMax)
			{
				assert(count < MAX_DEPTH);

				StackNode& stackNode = stack[count];
				stackNode.node = node;
				stackNode.tMin = tMin;
				stackNode.tMax = tMax;

				++count;
			}

			const StackNode& Pop()
			{
				assert(count > 0);

				--count;

				return stack[count];
			}

			bool IsEmpty() const { return count == 0; }
		};

		AABB bounds;
		uint32_t maxDepth;

		std::vector<SplitPosition> splitPositions;

		KDTreeNode<ElementType>* nodes;
		uint32_t lastNode;
		uint32_t availableNodes;

		ElementType** elementBuffer;
		uint32_t elementBufferSize;
		uint32_t elementBufferOffset;
		
	
	public:
		KDTree(uint32_t maxDepth);
		~KDTree();

		void Optimize(const AABB& bounds);
		void Analyze() const;

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance = FLT_MAX) const;

	private:
		void CreateNode(uint32_t nodeIdx, const AABB& bounds, const std::vector<ElementType*>& elements, int depth = 0);
		void InitialiseLeaf(KDTreeNode<ElementType>* node, const std::vector<ElementType*>& elements);

		bool IntersectRayRec(KDTreeNode<ElementType>* node, const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const;
		bool IntersectRaySec(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const;

		bool SplitSAH(const std::vector<ElementType*>& elements, const AABB& bounds, int& bestAxis, float& bestSplitPosition);
		bool SplitSAH(int axis, const std::vector<ElementType*>& elements, const AABB& bounds, float& bestSplitPosition, float& lowestCost);

		bool SplitFast(const std::vector<ElementType*>& elements, const AABB& bounds, int& bestAxis, float& bestSplitPosition);

		void CalculateBounds(const AABB& bounds, int axis, float splitPoint, AABB& upper, AABB& lower);

		static bool SortSplitPosition(const SplitPosition& a, const SplitPosition& b)
		{
			return a.position < b.position;
		}
	};

}

#endif