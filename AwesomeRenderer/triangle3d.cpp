#include "stdafx.h"
#include "awesomerenderer.h"

#include "triangle3d.h"

#include "plane.h"
#include "aabb.h"

using namespace AwesomeRenderer;

Triangle3D::Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c) :
	Triangle(a, b, c), Primitive()
{
	vO[0] = a; vO[1] = b; vO[2] = c;

	CalculateNormal();
	PreCalculateBarycentric();

	vN[0] = normal, vN[1] = normal; vN[2] = normal;
}

Triangle3D::Triangle3D(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& aN, const Vector3& bN, const Vector3& cN) :
	Triangle3D(a, b, c)
{
	vN[0] = aN; vN[1] = bN; vN[2] = cN;
}

Triangle3D::Triangle3D(const Triangle3D& other) :
	Triangle3D(other.v[0], other.v[1], other.v[2], other.vN[0], other.vN[1], other.vN[2])
{
	
}

const Vector3& Triangle3D::CalculateNormal()
{
	// Calculate normal as a vector perpendicular to ab and ac
	Vector3 ab = v[1] - v[0];
	Vector3 ac = v[2] - v[0];

	ab.normalize();
	ac.normalize();

	normal = cml::cross(ac, ab);
	normal.normalize();

	return normal;
}

void Triangle3D::Transform(const Matrix44& mtx)
{
	// Transform all vertices from object space by the given matrix
	v[0] = cml::transform_point(mtx, vO[0]);
	v[1] = cml::transform_point(mtx, vO[1]);
	v[2] = cml::transform_point(mtx, vO[2]);

	CalculateNormal();
	PreCalculateBarycentric();
}

bool Triangle3D::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	float distanceToPlane = cml::dot(normal, ray.origin - v[0]);
	float dot = -cml::dot(normal, ray.direction);

	// Ray is parallel to triangle plane
	if (fabs(dot) < 1e-3f)
		return false;

	float t = distanceToPlane / dot;

	// Intersection point is behind the ray
	if (t < 0.0f)
		return false;

	Vector3 pointOnPlane = ray.origin + t * ray.direction;

	// Calculate barycentric coords to check if the point is within triangle boundaries
	if (!IsPointInside(pointOnPlane, hitInfo.barycentricCoords))
		return false;

	// Fill the hit info struct with gathered data
	hitInfo.point = pointOnPlane;
	hitInfo.distance = t;

	VectorUtil<3>::Interpolate(vN[0], vN[1], vN[2], hitInfo.barycentricCoords, hitInfo.normal);
	
	hitInfo.inside = cml::dot(ray.direction, hitInfo.normal) >= 0;
	
	return true;
}

int Triangle3D::SideOfPlane(const Plane& plane) const
{
	int a = plane.SideOfPlane(v[0]);
	int b = plane.SideOfPlane(v[1]);
	int c = plane.SideOfPlane(v[2]);

	// If all vertices are on the same side, return that side
	// Otherwise the triangle intersects the plane
	return (a == b && a == c) ? a : 0;
}

void Triangle3D::CalculateBounds(AABB& bounds) const
{
	Vector3 lower, upper;
	CalculateBounds(lower, upper);

	bounds.Initialize(lower, upper);
}