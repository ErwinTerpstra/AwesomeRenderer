#ifndef _SHAPE_H_
#define _SHAPE_H_

namespace AwesomeRenderer
{
	class Plane;
	class AABB;

	class Shape : public virtual Object
	{

	public:


	public:
		Shape();

		virtual ~Shape() { }

		virtual void Transform(const Matrix44& mtx) = 0;

		virtual bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const = 0;

		virtual int SideOfPlane(const Plane& plane) const = 0;

		virtual void CalculateBounds(AABB& bounds) const = 0;

		virtual const Shape& GetShape() const { return *this; }
	};
}

#endif