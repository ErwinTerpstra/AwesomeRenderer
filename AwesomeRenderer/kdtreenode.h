#ifndef _KD_TREE_NODE_H_
#define _KD_TREE_NODE_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	template <typename ElementType>
	class KDTreeNode
	{
	private:
		// The lowest two bits always signifies the split axis. A split axis of 3 indicates this is a leaf node
		// For leaf nodes: The upper bytes are the number of elements in this leaf
		// For non-leaf nodes: Tthe upper bytes are the index of the upper element, the lower element is always allocated directly behind it
		uint32_t data;

		union
		{
			float splitPoint;
			ElementType** elementList;
		};

	public:
		KDTreeNode() : data(0)
		{

		}

		~KDTreeNode()
		{

		}

		void InitialiseLeaf(ElementType** elements, uint32_t elementCount)
		{
			data = (elementCount << 2) | 0x03;
			elementList = elements;
		}

		void InitialiseNonLeaf(float splitPoint, uint32_t axis, uint32_t upperNode)
		{
			this->splitPoint = splitPoint;
			data = (upperNode << 2) | axis;
		}


		float GetSplitPoint() const { return splitPoint; }

		AR_FORCE_INLINE bool IsLeaf() const { return (data & 0x03) == 0x03; }

		AR_FORCE_INLINE int GetAxis() const { return (int)(data & 0x03); }

		AR_FORCE_INLINE uint32_t GetUpperNode() const { return (data >> 2); }
		
		ElementType** GetElements() const { return elementList; }
		uint32_t GetElementCount() const { return (data >> 2); }
		
	private:

	};
}

#endif