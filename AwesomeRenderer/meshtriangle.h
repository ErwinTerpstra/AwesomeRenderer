#ifndef _MESH_TRIANGLE_H_
#define _MESH_TRIANGLE_H_

#include "awesomerenderer.h"
#include "triangle.h"
#include "primitive.h"

namespace AwesomeRenderer
{

	class Ray;
	class MeshEx;
	struct RaycastHit;
	
	class MeshTriangle : public Triangle<Vector3>, public Primitive
	{
		using Triangle<Vector3>::CalculateBounds;
		using Primitive::CalculateBounds;

	public:
		// Normal vector for this triangle (world space)
		Vector3 normal;

		uint32_t faceIdx;
	private:

	public:
		MeshTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32_t faceIdx);
		MeshTriangle(const MeshTriangle& other);

		const Vector3& CalculateNormal();
		
		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const;

		int SideOfPlane(const Plane& plane) const;
		int SideOfAAPlane(int axis, float position) const;

		void CalculateBounds(AABB& bounds) const;
		float Area() const;

	};
}

#endif