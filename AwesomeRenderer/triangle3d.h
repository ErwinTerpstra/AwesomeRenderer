#ifndef _TRIANGLE_3D_H_
#define _TRIANGLE_3D_H_

#include "awesomerenderer.h"
#include "triangle.h"
#include "primitive.h"

namespace AwesomeRenderer
{

	class Ray;
	struct RaycastHit;

	class Triangle3D : public Triangle<Vector3>, public Primitive
	{
		using Triangle<Vector3>::CalculateBounds;
		using Primitive::CalculateBounds;

	public:
		// Normal vector for this triangle (world space)
		Vector3 normal;

		// Vertex normals in object space
		Vector3 vN[3];

		// Vertices in object space
		Vector3 vO[3];

	private:

	public:
		Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c);
		Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& aN, const Vector3& bN, const Vector3& cN);
		Triangle3D(const Triangle3D& other);

		const Vector3& CalculateNormal();
		
		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const;

		int SideOfPlane(const Plane& plane) const;

		void CalculateBounds(AABB& bounds) const;
	};
}

#endif