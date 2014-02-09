#ifndef _TRIANGLE_3D_H_
#define _TRIANGLE_3D_H_

namespace AwesomeRenderer
{

	class Ray;
	struct RaycastHit;

	class Triangle3D : public Triangle<Vector3>
	{

	public:
		Vector3 normal;

	private:
		bool dirty;

	public:
		Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c);
		Triangle3D(const Triangle3D& other);

		const Vector3& CalculateNormal();
		
		void Transform(const Matrix44& mtx);
		
		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo);
	};
}

#endif