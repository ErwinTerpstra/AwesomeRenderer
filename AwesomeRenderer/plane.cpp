#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Plane::Plane(float d, const Vector3& normal) : d(d), normal(normal), dTransformed(d), normalTransformed(normal)
{

}

void Plane::Transform(const Matrix44& mtx)
{
	Vector3 transformedPoint = cml::transform_point(mtx, PointOnPlane());

	normalTransformed = cml::transform_vector(mtx, normal);
	normalTransformed.normalize();

	// Convert back to distance-from-origin representation
	dTransformed = cml::dot(transformedPoint, normalTransformed);
}

bool Plane::IntersectRay(const Ray& ray, RaycastHit& hitInfo)
{
	float dot = cml::dot(normalTransformed, ray.direction);

	// Ray is parallel to triangle plane
	if (dot == 0.0f)
		return false;

	float t = Distance(ray.origin) / dot;

	// Intersection point is behind the ray
	if (t < 0.0f)
		return false;

	Vector3 pointOnPlane = ray.origin + t * ray.direction;

	// Fill the hit info struct with gathered data
	hitInfo.point = pointOnPlane;
	hitInfo.distance = t;

	return true;
}

int Plane::SideOfPlane(const Vector3& point) const
{
	return (int) cml::sign(cml::dot(point - PointOnPlane(), normal));
}

int Plane::SideOfPlane(const Plane& plane) const
{
	// If the planes are perpendicular compare their distances from the origin
	// Otherwise they always intersect
	return (int) std::fabs(cml::dot(normalTransformed, plane.normalTransformed)) >= 1.0 - FLT_EPSILON ? cml::sign(plane.d - d) : 0;
}