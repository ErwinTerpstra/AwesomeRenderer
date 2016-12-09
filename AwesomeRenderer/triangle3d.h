#ifndef _TRIANGLE_3D_H_
#define _TRIANGLE_3D_H_

#include "awesomerenderer.h"
#include "triangle.h"
#include "primitive.h"

namespace AwesomeRenderer
{

	class Ray;
	class MeshEx;
	struct RaycastHit;
	
	class Triangle3D : public Triangle<Vector3>, public Primitive
	{
		using Triangle<Vector3>::CalculateBounds;
		using Primitive::CalculateBounds;

	public:
		// Normal vector for this triangle (world space)
		Vector3 normal;

		uint32_t vIdx[3];
	private:

	public:
		Triangle3D(const MeshEx& mesh, uint32_t vIdx0, uint32_t vIdx1, uint32_t vIdx2);
		Triangle3D(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32_t vIdx0, uint32_t vIdx1, uint32_t vIdx2);
		Triangle3D(const Triangle3D& other);

		const Vector3& CalculateNormal();
		
		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const;

		int SideOfPlane(const Plane& plane) const;
		int SideOfAAPlane(int axis, float position) const;

		void CalculateBounds(AABB& bounds) const;

	};
}

#endif