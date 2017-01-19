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
	hitInfo.normal = VectorUtil<3>::Normalize(hitInfo.point - centerTransformed);
	
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

float Sphere::Area() const
{
	return 4.0f * PI * radiusTransformed * radiusTransformed;
}


Vector3 Sphere::Sample(const Vector2& r, Vector3& normal) const
{
	normal = UniformSample(r);
	return centerTransformed + normal * radiusTransformed;
}

Vector3 Sphere::Sample(const Vector3& p, const Vector2& r, Vector3& normal) const
{
	Vector3 toCenter = (centerTransformed - p);
	float distanceSq = toCenter.length_squared();
	float r2 = radiusTransformed * radiusTransformed;

	// If the origin point is inside the sphere, sample the whole sphere uniformly
	if (distanceSq - r2 < 1e-3f)
		return Sample(r, normal);

	// Calculate the maximum subtended angle for the cone
	float sinThetaMax2 = r2 / distanceSq;
	float cosThetaMax = sqrtf(std::max(0.0f, 1.0f - sinThetaMax2));

	// Create a coordinate system around the vector from the point to the sphere center
	Vector3 up = toCenter / sqrtf(distanceSq);
	Vector3 right, forward;
	VectorUtil<3>::OrthoNormalize(up, right, forward);

	Vector3 ps;

	// Create a ray with an random distribution inside the cone
	Ray ray(p, UniformSampleCone(r, cosThetaMax, right, up, forward));
	RaycastHit hitInfo;
	if (IntersectRay(ray, hitInfo))
		ps = hitInfo.point;
	else
		ps = ray.origin + ray.direction * VectorUtil<3>::Dot(toCenter, ray.direction);

	normal = VectorUtil<3>::Normalize(ps - centerTransformed);
	return ps;
}

float Sphere::CalculatePDF(const Vector3& p, const Vector3& wi) const
{
	Vector3 fromCenter = (p - centerTransformed);
	float distanceSq = fromCenter.length_squared();
	float r2 = radiusTransformed * radiusTransformed;
	if (distanceSq - r2 < 1e-3f)
		return Primitive::CalculatePDF(p, wi);

	float sinThetaMax2 = r2 / distanceSq;
	float cosThetaMax = sqrtf(std::max(0.0f, 1.0f - sinThetaMax2));

	return UniformConePDF(cosThetaMax);
}

Vector3 Sphere::UniformSample(const Vector2& r)
{
	float cosTheta = 1.0f - 2.0f * r[0];
	float sinTheta = sqrtf(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = 2.0f * PI * r[1];

	return Vector3(sinTheta * cosf(phi), cosTheta, sinTheta * sinf(phi));
}

float Sphere::UniformConePDF(float cosThetaMax)
{
	return 1.f / (2.f * PI * (1.f - cosThetaMax));
}


Vector3 Sphere::UniformSampleCone(const Vector2& r, float cosThetaMax)
{
	float cosTheta = (1.f - r[0]) + r[0] * cosThetaMax;
	float sinTheta = sqrtf(1.f - cosTheta * cosTheta);
	float phi = r[1] * 2.f * PI;
	return Vector3(cosf(phi) * sinTheta, cosTheta, sinf(phi) * sinTheta);
}


Vector3 Sphere::UniformSampleCone(const Vector2& r, float cosThetaMax, const Vector3& x, const Vector3& y, const Vector3& z)
{
	Vector3 sample = UniformSampleCone(r, cosThetaMax);
	return sample[0] * x + sample[1] * y + sample[2] * z;
}