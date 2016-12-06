#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "awesomerenderer.h"
#include "primitive.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Plane;
	class AABB;

	class Sphere : public Primitive
	{

	private:
		Vector3 center;
		Vector3 centerTransformed;
		
		float radius;
		float radiusTransformed;

	public:
		Sphere();
		Sphere(const Vector3& center, float radius);
		
		void Initialize(const Vector3& center, float radius);

		void Transform(const Matrix44& mtx);
		
		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const;

		int SideOfPlane(const Plane& plane) const;

		void CalculateBounds(AABB& bounds) const;

		const Vector3& Center() { return centerTransformed; }
		const float& Radius() { return radiusTransformed; }
	};
}

#endif