#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Sphere::Sphere() : center(), centerTransformed()
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
		
bool Sphere::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	// Compute A, B and C coefficients
	float a = cml::dot(ray.direction, ray.direction);
	float b = 2 * cml::dot(ray.direction, ray.origin);
	float c = cml::dot(ray.origin, ray.origin) - (radiusTransformed * radiusTransformed);

    // Find discriminant
    float disc = b * b - 4 * a * c;
    
    // If discriminant is negative there are no real roots, so the ray misses the spher
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
    {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

	// If t0 is smaller than zero, both intersections are in negative distance. So the ray misses
	if (t0 < 0.0f)
		return false;

	// TODO: If the origin of the ray is inside the sphere, this point where the ray leaves the sphere is returned
	// Is this the desired behaviour?

	// Fill RaycastHit struct
	hitInfo.distance = t0;
	hitInfo.point = ray.origin + ray.direction * t0;
	
	return true;
}

int Sphere::SideOfPlane(const Plane& plane) const
{
	// The plane intersects the sphere if the shortest distance from the center to the plane is smaller than the radius
	// Otherwise the sphere is on the side the center is located
	return plane.Distance(centerTransformed) < radiusTransformed ? 0 : plane.SideOfPlane(centerTransformed);
}