#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "treeelement.h"

namespace AwesomeRenderer
{
	class Primitive;
	class Ray;

	struct RaycastHit;

	class Shape : public TreeElement
	{
	public:
		virtual ~Shape() { }

		virtual void Transform(const Matrix44& mtx) = 0;

		virtual bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const = 0;

		virtual const Primitive& GetPrimitive() const = 0;

	};

}

#endif