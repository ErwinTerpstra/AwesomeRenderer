#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include "awesomerenderer.h"
#include "shape.h"

#include "ray.h"
#include "raycasthit.h"

namespace AwesomeRenderer
{
	class Plane;
	class AABB;

	class Primitive : public virtual Shape
	{

	public:


	public:
		Primitive();

		virtual ~Primitive() { }

		virtual int SideOfPlane(const Plane& plane) const = 0;

		virtual void CalculateBounds(AABB& bounds) const = 0;

		virtual const Primitive& GetPrimitive() const { return *this; }
	};
}

#endif