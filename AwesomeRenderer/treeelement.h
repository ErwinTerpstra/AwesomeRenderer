#ifndef _TREE_ELEMENT_H_
#define _TREE_ELEMENT_H_

namespace AwesomeRenderer
{
	class Primitive;

	class TreeElement
	{
		public:
			virtual const Primitive& GetPrimitive() const = 0;
	};

}

#endif