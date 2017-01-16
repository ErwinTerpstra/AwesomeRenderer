#include "stdafx.h"

#include "awesomerenderer.h"

#include "quad.h"
#include "aabb.h"

using namespace AwesomeRenderer;

Quad::Quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) : upper(c, b, a), lower(a, d, c)
{

}

void Quad::Transform(const Matrix44& mtx)
{
	upper.Transform(mtx);
	lower.Transform(mtx);
}

bool Quad::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	if (upper.IntersectRay(ray, hitInfo, maxDistance))
	{
		lower.IntersectRay(ray, hitInfo, hitInfo.distance);
		return true;
	}

	return lower.IntersectRay(ray, hitInfo, maxDistance);
}

int Quad::SideOfPlane(const Plane& plane) const
{
	int upperSide = upper.SideOfPlane(plane);
	int lowerSide = lower.SideOfPlane(plane);

	if (upperSide == lowerSide)
		return upperSide;
	
	return 0;
}

void Quad::CalculateBounds(AABB& bounds) const
{
	AABB upperBounds, lowerBounds;
	upper.CalculateBounds(upperBounds);
	lower.CalculateBounds(lowerBounds);

	Vector3 upperMin = upperBounds.Min();
	Vector3 upperMax = upperBounds.Max();

	Vector3 lowerMin = lowerBounds.Min();
	Vector3 lowerMax = lowerBounds.Max();

	Vector3 min(std::min(upperMin[0], lowerMin[0]), std::min(upperMin[1], lowerMin[1]), std::min(upperMin[2], lowerMin[2]));
	Vector3 max(std::min(upperMax[0], lowerMax[0]), std::min(upperMax[1], lowerMax[1]), std::min(upperMax[2], lowerMax[2]));

	bounds.Initialize(min, max);
}

float Quad::Area() const
{
	return upper.Area() + lower.Area();
}
