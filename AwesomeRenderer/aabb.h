#ifndef _AABB_H_
#define _AABB_H_

namespace AwesomeRenderer
{

	class AABB : public Shape
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

		const Vector3& Min() const { return min; }
		const Vector3& Max() const { return max; }

	};

}

#endif