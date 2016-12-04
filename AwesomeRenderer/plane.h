#ifndef _PLANE_H_
#define _PLANE_H_

#include "awesomerenderer.h"
#include "primitive.h"

namespace AwesomeRenderer
{
	class Plane : public Primitive
	{

	private:
		float d;
		float dTransformed;

		Vector3 normal;
		Vector3 normalTransformed;

	public:
		Plane(float d, const Vector3& normal);

		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const;

		int SideOfPlane(const Plane& plane) const;
		int SideOfPlane(const Vector3& point) const;

		void CalculateBounds(AABB& bounds) const { assert(false && "Can't calculate bounds for an infinite plane!"); }

		__inline Vector3 PointOnPlane() const { return dTransformed * normalTransformed; }
		__inline float Distance(const Vector3& point) const 
		{
			Vector3 delta = point - PointOnPlane();
			return cml::dot(normalTransformed, delta);
		}

	};
}

#endif