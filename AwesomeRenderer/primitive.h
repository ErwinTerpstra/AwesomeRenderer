#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include "awesomerenderer.h"
#include "object.h"

#include "ray.h"
#include "raycasthit.h"

namespace AwesomeRenderer
{
	class Plane;
	class AABB;

	class Primitive : public virtual Object
	{

	public:


	public:
		Primitive();

		virtual ~Primitive() { }

		virtual void Transform(const Matrix44& mtx) = 0;

		virtual bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const = 0;

		virtual int SideOfPlane(const Plane& plane) const = 0;

		virtual void CalculateBounds(AABB& bounds) const = 0;

		virtual const Primitive& GetShape() const { return *this; }
	};
}

#endif