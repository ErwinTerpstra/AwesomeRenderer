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
		virtual int SideOfAAPlane(int axis, float position) const;

		virtual void CalculateBounds(AABB& bounds) const = 0;
		virtual float Area() const = 0;

		virtual Vector3 Sample(const Vector2& r, Vector3& normal) const
		{
			assert(false && "Trying to sample a primitive which does not support sampling");

			return Vector3();
		}

		virtual Vector3 Sample(const Vector3& p, const Vector2& r, Vector3& normal) const
		{
			return Sample(r, normal);
		}

		virtual float CalculatePDF(const Vector3& p) const { return 1.0f / Area(); }
		virtual float CalculatePDF(const Vector3& p, const Vector3& wi) const
		{
			Ray ray(p, wi);

			RaycastHit hitInfo;
			if (!IntersectRay(ray, hitInfo))
				return 0.0f;

			float projectedArea = VectorUtil<3>::Dot(hitInfo.normal, -wi) * Area();
			if (projectedArea < 1e-3f)
				return 0.0f;

			float pdf = (hitInfo.point - p).length_squared() / projectedArea;

			return pdf;
		}

		virtual const Primitive& GetPrimitive() const { return *this; }
	};
}

#endif