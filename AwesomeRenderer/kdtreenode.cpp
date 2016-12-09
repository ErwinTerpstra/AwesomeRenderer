#include "stdafx.h"
#include "kdtreenode.h"
#include "kdtree.h"
#include "treeelement.h"

using namespace AwesomeRenderer;

KDTreeNode::KDTreeNode(ElementList* elements, KDTreeNode* parent) : data(0)
{
	SetLeaf(true);

	if (parent)
		SetAxis((parent->GetAxis() + 1) % 3);
	else
		SetAxis(0);

	SetElements(*elements);
}

KDTreeNode::~KDTreeNode()
{

}
