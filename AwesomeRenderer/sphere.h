#ifndef _SPHERE_H_
#define _SPHERE_H_

namespace AwesomeRenderer
{
	class Sphere : public Shape
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
		
		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const;

		int SideOfPlane(const Plane& plane) const;

		void CalculateBounds(AABB& bounds) const 
		{
			Vector3 extents(radiusTransformed, radiusTransformed, radiusTransformed);
			bounds.Initialize(centerTransformed - extents, centerTransformed + extents);
		}

		const Vector3& Center() { return centerTransformed; }
		const float& Radius() { return radiusTransformed; }
	};
}

#endif