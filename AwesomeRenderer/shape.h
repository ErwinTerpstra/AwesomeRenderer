#ifndef _SHAPE_H_
#define _SHAPE_H_

namespace AwesomeRenderer
{
	class Shape
	{

	public:


	public:
		Shape();

		virtual void Transform(const Matrix44& mtx) = 0;

		virtual bool IntersectRay(const Ray& ray, RaycastHit& hitInfo) const = 0;
	};
}

#endif