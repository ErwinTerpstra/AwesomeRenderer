#ifndef _NODE_H_
#define _NODE_H_

namespace AwesomeRenderer
{

	class Node : public Object
	{

	public:
		Model model;
		Transformation transform;


	public:
		Node();

		virtual const Shape& GetShape() const { return model.bounds; }

	};

}

#endif