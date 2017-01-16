#include "stdafx.h"
#include "awesomerenderer.h"
#include "aabb.h"

using namespace AwesomeRenderer;

AABB::AABB() : Primitive(), min(), max(), minTransformed(), maxTransformed()
{

}

AABB::AABB(const Vector3& min, const Vector3& max)
{
	Initialize(min, max);
}

void AABB::Initialize(const Vector3& min, const Vector3& max)
{
	this->min = min;
	this->max = max;

	minTransformed = min;
	maxTransformed = max;
}

void AABB::Transform(const Matrix44& mtx)
{
	// Retrieve all corner points of the cube
	Vector3 corners[8];
	GetCorners(corners);

	// Initialize transformed boundaries to extreme values
	minTransformed.set(FLT_MAX, FLT_MAX, FLT_MAX);
	maxTransformed.set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// Iterate through all corners to find the bounding values
	for (int cCorner = 0; cCorner < 8; ++cCorner)
	{
		Vector3 corner = cml::transform_point(mtx, corners[cCorner]);

		// Find the max/min values for each axis
		// This could be in a seperate loop but I think this is significantly faster
		minTransformed[0] = std::min(corner[0], minTransformed[0]);
		minTransformed[1] = std::min(corner[1], minTransformed[1]);
		minTransformed[2] = std::min(corner[2], minTransformed[2]);

		maxTransformed[0] = std::max(corner[0], maxTransformed[0]);
		maxTransformed[1] = std::max(corner[1], maxTransformed[1]);
		maxTransformed[2] = std::max(corner[2], maxTransformed[2]);
	}
}

void AABB::GetCorners(Vector3* corners) const
{
	GetCorners(corners, min, max);
}

void AABB::GetTransformedCorners(Vector3* corners) const
{
	GetCorners(corners, minTransformed, maxTransformed);
}

void AABB::GetCorners(Vector3* corners, const Vector3& min, const Vector3& max)
{
	corners[0] = Vector3(min[0], min[1], min[2]);
	corners[1] = Vector3(max[0], min[1], min[2]);
	corners[2] = Vector3(min[0], max[1], min[2]);
	corners[3] = Vector3(max[0], max[1], min[2]);

	corners[4] = Vector3(min[0], min[1], max[2]);
	corners[5] = Vector3(max[0], min[1], max[2]);
	corners[6] = Vector3(min[0], max[1], max[2]);
	corners[7] = Vector3(max[0], max[1], max[2]);
}

bool AABB::IntersectRay(const Ray& ray, float& tMin, float& tMax) const
{
	// Find the shortest intersection between the ray and an axis of the box
	float cMin = (minTransformed[0] - ray.origin[0]) * ray.invDirection[0];
	float cMax = (maxTransformed[0] - ray.origin[0]) * ray.invDirection[0];

	tMin = std::min(cMin, cMax);
	tMax = std::max(cMin, cMax);

	for (int axis = 1; axis < 3; ++axis)
	{
		cMin = (minTransformed[axis] - ray.origin[axis]) * ray.invDirection[axis];
		cMax = (maxTransformed[axis] - ray.origin[axis]) * ray.invDirection[axis];

		// Correct NaN handling
		//tMin = std::max(tMin, std::min(std::min(cMin, cMax), tMax));
		//tMax = std::min(tMax, std::max(std::max(cMin, cMax), tMin));

		tMin = std::max(tMin, std::min(cMin, cMax));
		tMax = std::min(tMax, std::max(cMin, cMax));
	}

	return tMax > std::max(tMin, 0.0f);
}

bool AABB::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	float tMin, tMax;
	if (!IntersectRay(ray, tMin, tMax))
		return false;

	if (tMin > maxDistance)
		return false;
	
	hitInfo.distance = tMin;
	hitInfo.point = ray.origin + ray.direction * tMin;

	// Decide the normal by checking which on which plane the hitpoint lies
	Vector3 normal(0.0f, 0.0f, 0.0f);
	for (int axis = 0; axis < 3; ++axis)
	{
		if (fabs(hitInfo.point[axis] - minTransformed[axis]) < 1e-5f)
		{
			normal[axis] = -1.0f;
			break;
		}

		if (fabs(hitInfo.point[axis] - maxTransformed[axis]) < 1e-5f)
		{
			normal[axis] = 1.0f;
			break;
		}
	}

	hitInfo.normal = normal;

	return true;
}

bool AABB::Contains(const Vector3& point) const
{
	return point[0] >= minTransformed[0] && point[1] >= minTransformed[1] && point[2] >= minTransformed[2] &&
		point[0] <= maxTransformed[0] && point[1] <= maxTransformed[1] && point[2] <= maxTransformed[2];
}

int AABB::SideOfPlane(const Plane& plane) const
{
	// Retrieve all corners of cube
	Vector3 corners[8];
	GetTransformedCorners(corners);

	// Retrieve side of first corner, if all other corners are on the same side, the entire box is on that side
	int side = plane.SideOfPlane(corners[0]);

	for (int cCorner = 1; cCorner < 8; ++cCorner)
	{
		// If at least two corners are on different sides, the box intersects the plane
		if (plane.SideOfPlane(corners[cCorner]) != side)
			return 0;
	}

	return side;
}

float AABB::Area() const
{
	return 2 * (Width() * Height() + Width() * Depth() + Height() * Depth());
}