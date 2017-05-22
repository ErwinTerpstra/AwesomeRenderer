#include "stdafx.h"
#include "kdtreenode.h"
#include "kdtree.h"
#include "treeelement.h"

using namespace AwesomeRenderer;

KDTreeNode::KDTreeNode() : data(0)
{
}

KDTreeNode::~KDTreeNode()
{

}

void KDTreeNode::InitialiseLeaf(TreeElement** elements, uint32_t elementCount)
{
	data = (elementCount << 2) | 0x03;
	elementList = elements;
}

void KDTreeNode::InitialiseNonLeaf(float splitPoint, uint32_t axis, uint32_t upperNode)
{
	this->splitPoint = splitPoint;
	data = (upperNode << 2) | axis;
}
