
#include "awesomerenderer.h"

#include "meshtriangle.h"

#include "plane.h"
#include "aabb.h"

#include "meshex.h"

using namespace AwesomeRenderer;

MeshTriangle::MeshTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32_t faceIdx) : Triangle(v0, v1, v2), Primitive(), faceIdx(faceIdx)
{
	CalculateNormal();
	PreCalculateBarycentric();
}

MeshTriangle::MeshTriangle(const MeshTriangle& other) :
	MeshTriangle(other.v[0], other.v[1], other.v[2], other.faceIdx)
{

}

const Vector3& MeshTriangle::CalculateNormal()
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

void MeshTriangle::Transform(const Matrix44& mtx)
{
	// TODO: Find a way to implement this without having to store additional data, which makes the path tracer slow
	// Or make it possible to inherit from Shape without having to implement this method
	assert(false && "Not implemented.");
}

bool MeshTriangle::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	float dot = VectorUtil<3>::Dot(normal, ray.direction);

	// Ray is parallel or hits the triangle plane from behind
	// TODO: Provide backface intersection?
	if (dot >= 0.0f)
		return false;

	float distanceToPlane = VectorUtil<3>::Dot(normal, ray.origin - v[0]);

	// Intersection point is behind the ray
	if (distanceToPlane < 0.0f)
		return false;

	float t = distanceToPlane / -dot;

	if (t > maxDistance)
		return false;

	Vector3 pointOnPlane = ray.origin + t * ray.direction;

	// Calculate barycentric coords to check if the point is within triangle boundaries
	Vector3 v2 = pointOnPlane - v[0];

	// Compute dot products
	float dot02 = VectorUtil<3>::Dot(v0, v2);
	float dot12 = VectorUtil<3>::Dot(v1, v2);

	// Compute barycentric coordinates
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	if (u < 0.0f || v < 0.0f || u + v > 1.0f)
		return false;
		
	// Fill the hit info struct with gathered data
	hitInfo.point = pointOnPlane;
	hitInfo.distance = t;
	hitInfo.barycentricCoords.set(1.0f - u - v, u, v);
		
	return true;
}

int MeshTriangle::SideOfPlane(const Plane& plane) const
{
	int a = plane.SideOfPlane(v[0]);
	int b = plane.SideOfPlane(v[1]);
	int c = plane.SideOfPlane(v[2]);

	// If all vertices are on the same side, return that side
	// Otherwise the triangle intersects the plane
	return (a == b && a == c) ? a : 0;
}

int MeshTriangle::SideOfAAPlane(int axis, float position) const
{
	int a = Util::Sign(v[0][axis] - position);
	int b = Util::Sign(v[1][axis] - position);
	int c = Util::Sign(v[2][axis] - position);

	// If all vertices are on the same side, return that side
	// Otherwise the triangle intersects the plane
	return (a == b && a == c) ? a : 0;
}

void MeshTriangle::CalculateBounds(AABB& bounds) const
{
	Vector3 lower, upper;
	CalculateBounds(lower, upper);

	bounds.Initialize(lower, upper);
}

float MeshTriangle::Area() const
{
	Vector3 p = v[0] + (dot01 * VectorUtil<3>::Normalize(v1));

	return 0.5f * v1.length() * (v[1] - p).length();
}