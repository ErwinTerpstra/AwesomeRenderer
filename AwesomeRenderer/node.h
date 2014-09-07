#ifndef _NODE_H_
#define _NODE_H_

namespace AwesomeRenderer
{
	class Model;
	class Transformation;

	class NodeEx;

	class Node : public Object, public Extendee<NodeEx>
	{

	public:
		Model* model;
		Transformation* transform;

	public:
		Node();

		virtual const Shape& GetShape() const;

	};

}

#endif