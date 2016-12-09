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
		typedef std::vector<const TreeElement*> ElementList;

	private:
		// For leaf nodes: this is a pointer to the elements list
		// For non-leaf nodes: this is a pointer to the upper element, the lower element is always allocated directly behind it
		// The lower 3 bits of the pointer are not used since nodes are 8 byte aligned
		// The two LSBs contain the split axis and the third LSB contains a flag whether this is a leaf
		uint32_t data;

		float splitPoint;

	public:
		KDTreeNode(ElementList* elements, KDTreeNode* parent = NULL);

		~KDTreeNode();

		float GetSplitPoint() const { return splitPoint; }
		void SetSplitPoint(float splitPoint) { this->splitPoint = splitPoint; }

		AR_FORCE_INLINE bool IsLeaf() const { return (data & 4) > 0; }
		AR_FORCE_INLINE void SetLeaf(bool leaf) { data = leaf ? (data | 4) : (data & 0xfffffffb); }
		
		AR_FORCE_INLINE int GetAxis() const { return (int)(data & 3); }
		AR_FORCE_INLINE void SetAxis(int axis) { data = (data & 0xfffffffc) + axis; }

		AR_FORCE_INLINE KDTreeNode* GetUpperNode() const { return (KDTreeNode*) (data & 0xfffffff8); }
		AR_FORCE_INLINE KDTreeNode* GetLowerNode() const { return GetUpperNode() + 1; }

		void SetUpperNode(KDTreeNode* node) { data = ((uint32_t)node) + (data & 7); }

		ElementList& GetElements() const { return *(ElementList*)(data & 0xfffffff8); }
		void SetElements(ElementList& list) { data = ((uint32_t)&list) + (data & 7); }

	private:

	};
}

#endif