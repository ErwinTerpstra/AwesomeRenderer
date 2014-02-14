#ifndef _KD_TREE_H_
#define _KD_TREE_H_

namespace AwesomeRenderer
{
	class Object;

	class KDTree
	{

	public:
		static const int MAX_NODES_PER_LEAF;
		static const int MAX_DEPTH;

		KDTree *leftNode, *rightNode;

		std::vector<Object*> objects;

	private:
		KDTree *parent;

		int axis;

		float splitPoint;

	public:

		KDTree(KDTree* parent);
		~KDTree();

		void Optimize(int depth = 0);

		bool IsLeaf() const { return leftNode == NULL && rightNode == NULL; }

	private:
		void Split();

	};
}

#endif