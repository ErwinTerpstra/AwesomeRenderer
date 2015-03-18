#ifndef _AABB_H_
#define _AABB_H_

#include "awesomerenderer.h"
#include "primitive.h"
#include "plane.h"

namespace AwesomeRenderer
{

	class AABB : public Primitive
	{

	private:

		Vector3 min, max;
		Vector3 minTransformed, maxTransformed;

	public:

		AABB();
		AABB(const Vector3& min, const Vector3& max);

		void Initialize(const Vector3& min, const Vector3& max);

		void Transform(const Matrix44& mtx);
		
		void GetCorners(Vector3* corners) const;
		void GetTransformedCorners(Vector3* corners) const;

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const;
		
		int SideOfPlane(const Plane& plane) const;

		void CalculateBounds(AABB& bounds) const { bounds.Initialize(minTransformed, maxTransformed); }

		float Width() const { return maxTransformed[0] - minTransformed[0]; }
		float Height() const { return maxTransformed[1] - minTransformed[1]; }
		float Depth() const { return maxTransformed[2] - minTransformed[2]; }

		const Vector3& Min() const { return minTransformed; }
		const Vector3& Max() const { return maxTransformed; }

	private:
		static void GetCorners(Vector3* corners, const Vector3& min, const Vector3& max);

	};

}

#endif