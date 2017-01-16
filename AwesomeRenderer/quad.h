#ifndef _QUAD_H_
#define _QUAD_H_

#include "primitive.h"
#include "triangle3d.h"

namespace AwesomeRenderer
{
	class Quad : public Primitive
	{

	private:
		Triangle3D upper, lower;

	public:
		Quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);

		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const;

		int SideOfPlane(const Plane& plane) const;

		void CalculateBounds(AABB& bounds) const;
		float Area() const;
		
		static Quad* CreateUnitQuad()
		{
			return new Quad(Vector3(-0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.0f, -0.5f), Vector3(-0.5f, 0.0f, -0.5f));
		}
	};
}

#endif