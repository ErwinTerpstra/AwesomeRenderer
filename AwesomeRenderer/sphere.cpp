#include "stdafx.h"
#include "awesomerenderer.h"

#include "sphere.h"
#include "ray.h"
#include "raycasthit.h"
#include "primitive.h"
#include "plane.h"
#include "aabb.h"

using namespace AwesomeRenderer;

Sphere::Sphere() : Primitive(), center(), centerTransformed()
{

}

Sphere::Sphere(const Vector3& center, float radius)
{
	Initialize(center, radius);
}

void Sphere::Initialize(const Vector3& center, float radius)
{
	this->center = center;
	this->radius = radius;

	centerTransformed = center;
	radiusTransformed = radius;
}

void Sphere::Transform(const Matrix44& mtx)
{
	// Transform a uniform scale to find out the maximum scaled dimension
	// TODO: Is this the best/fastest way to retrieve the scale from a matrix?
	Vector3 scale = cml::transform_vector(mtx, Vector3(1.0f, 1.0f, 1.0f));

	radiusTransformed = radius * std::max(scale[0], std::max(scale[1], scale[2]));
	centerTransformed = cml::transform_point(mtx, center);
}
		
bool Sphere::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	Vector3 relativeOrigin = ray.origin - centerTransformed;

	// Compute A, B and C coefficients
	float a = VectorUtil<3>::Dot(ray.direction, ray.direction);
	float b = 2 * VectorUtil<3>::Dot(ray.direction, relativeOrigin);
	float c = VectorUtil<3>::Dot(relativeOrigin, relativeOrigin) - (radiusTransformed * radiusTransformed);

    // Find discriminant
    float disc = b * b - 4 * a * c;
    
    // If discriminant is negative there are no real roots, so the ray misses the sphere
    if (disc < 0)
        return false;

	// Compute distance on intersection
    float distSqrt = std::sqrtf(disc);
    float q;

    if (b < 0)
        q = (-b - distSqrt) / 2.0f;
    else
        q = (-b + distSqrt) / 2.0f;

    // Compute t0 and t1
    float t0 = q / a;
    float t1 = c / q;

	// Check if t0 is smaller than t1. Otherwise, swap them around
	if (t0 > t1)
		std::swap(t0, t1);

	// Check if t0 is in negative time
	if (t0 < 0.0f)
		std::swap(t0, t1);

	// If t0 is still smaller than zero, both intersections are in negative time. So the ray misses
	if (t0 < 0.0f || t0 > maxDistance)
		return false;

	// Fill RaycastHit struct
	hitInfo.distance = t0;
	hitInfo.point = ray.origin + ray.direction * t0;
	hitInfo.normal = cml::normalize(hitInfo.point - centerTransformed);
	
	return true;
}

int Sphere::SideOfPlane(const Plane& plane) const
{
	// The plane intersects the sphere if the shortest distance from the center to the plane is smaller than the radius
	// Otherwise the sphere is on the side the center is located
	return plane.Distance(centerTransformed) < radiusTransformed ? 0 : plane.SideOfPlane(centerTransformed);
}

void Sphere::CalculateBounds(AABB& bounds) const
{
	Vector3 extents(radiusTransformed, radiusTransformed, radiusTransformed);
	bounds.Initialize(centerTransformed - extents, centerTransformed + extents);
}