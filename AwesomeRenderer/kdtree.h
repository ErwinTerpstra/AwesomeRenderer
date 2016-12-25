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
		static const uint32_t MAX_DEPTH = 32;
		static const float TRAVERSAL_COST;
		static const float INTERSECTION_COST;
		static const float POSITION_EPSILON;

		KDTreeNode* rootNode;

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
			KDTreeNode* node;
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

			void Push(KDTreeNode* node, float tMin, float tMax)
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
	
	public:
		KDTree(uint32_t maxDepth);
		~KDTree();

		void Optimize(const AABB& bounds);
		void Analyze() const;

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance = FLT_MAX) const;

	private:
		void Optimize(KDTreeNode* node, const AABB& bounds, int depth = 0);

		bool IntersectRayRec(KDTreeNode* node, const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const;
		bool IntersectRaySec(const Ray& ray, RaycastHit& hitInfo, float tMin, float tMax) const;

		bool SplitSAH(KDTreeNode* node, const AABB& bounds);
		void SplitSAH(KDTreeNode* node, int axis, const AABB& bounds, float& bestSplitPosition, float& lowestCost);

		void SplitMode(KDTreeNode* node, const AABB& bounds);
		bool SplitFast(KDTreeNode* node, const AABB& bounds);

		void CalculateBounds(const AABB& bounds, int axis, float splitPoint, AABB& upper, AABB& lower);

		template<typename T>
		T* Allocate(uint32_t count = 1);

		template<typename T>
		void Free(T* block);

		static bool SortSplitPosition(const SplitPosition& a, const SplitPosition& b)
		{
			return a.position < b.position;
		}
	};
}

#endif