#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Triangle3D::Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c) :
	Triangle(a, b, c), dirty(true)
{

}

Triangle3D::Triangle3D(const Triangle3D& other) :
	Triangle3D(other.v[0], other.v[1], other.v[2])
{
	
}

const Vector3& Triangle3D::CalculateNormal()
{
	if (!dirty)
		return normal;

	Vector3 ab = v[1] - v[0];
	Vector3 ac = v[2] - v[0];

	ab.normalize();
	ac.normalize();

	dirty = false;

	return normal = cml::cross(ac, ab);
}

void Triangle3D::Transform(const Matrix44& mtx)
{
	v[0] = cml::transform_point(mtx, v[0]);
	v[1] = cml::transform_point(mtx, v[1]);
	v[2] = cml::transform_point(mtx, v[2]);

	dirty = true;
}

bool Triangle3D::IntersectRay(const Ray& ray, RaycastHit& hitInfo)
{
	// Make sure our normal vector is up to date
	CalculateNormal();

	float d = cml::dot(normal, v[0]);
	float distanceToPlane = -(cml::dot(normal, ray.origin) + d);

	float dot = cml::dot(normal, ray.direction);

	// Ray is parallel to triangle plane
	if (dot == 0.0f)
		return false;

	float t = distanceToPlane / dot;

	// Intersection point is behind the ray
	if (t < 0.0f)
		return false;

	Vector3 pointOnPlane = ray.origin + t * ray.direction;

	if (!IsPointInside(pointOnPlane, hitInfo.barycentricCoords))
		return false;

	hitInfo.point = pointOnPlane;
	hitInfo.distance = t;

	return true;
}
