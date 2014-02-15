#ifndef _OBJECT_H_
#define _OBJECT_H_

namespace AwesomeRenderer
{
	class Shape;

	class Object
	{
	public:
		virtual ~Object() { }
		virtual const Shape& GetShape() const = 0;
	};

}

#endif