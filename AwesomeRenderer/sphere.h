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
		float Area() const;

		Vector3 Sample(const Vector2& r, Vector3& normal) const;

		Vector3 Sample(const Vector3& p, const Vector2& r, Vector3& normal) const;

		float CalculatePDF(const Vector3& p, const Vector3& wi) const;

		const Vector3& Center() { return centerTransformed; }
		const float& Radius() { return radiusTransformed; }

		static Vector3 UniformSample(const Vector2& r);
	};
}

#endif