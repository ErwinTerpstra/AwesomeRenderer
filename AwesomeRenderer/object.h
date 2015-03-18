#ifndef _OBJECT_H_
#define _OBJECT_H_

namespace AwesomeRenderer
{
	class Primitive;

	class Object
	{
	public:
		virtual ~Object() { }
		virtual const Primitive& GetShape() const = 0;
	};

}

#endif