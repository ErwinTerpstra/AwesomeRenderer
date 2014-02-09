#include "awesomerenderer.h"

using namespace AwesomeRenderer;


AABB::AABB() : min(), max(), minTransformed(), maxTransformed()
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
	// Transform all corner points of the cube
	Vector3 corners[] = 
	{
		cml::transform_point(mtx, Vector3(min[0], min[1], min[2])),
		cml::transform_point(mtx, Vector3(max[0], min[1], min[2])),
		cml::transform_point(mtx, Vector3(min[0], max[1], min[2])),
		cml::transform_point(mtx, Vector3(max[0], max[1], min[2])),
		
		cml::transform_point(mtx, Vector3(min[0], min[1], max[2])),
		cml::transform_point(mtx, Vector3(max[0], min[1], max[2])),
		cml::transform_point(mtx, Vector3(min[0], max[1], max[2])),
		cml::transform_point(mtx, Vector3(max[0], max[1], max[2])),
	};

	// Initialize transformed boundaries to extreme values
	minTransformed.set(FLT_MAX, FLT_MAX, FLT_MAX);
	maxTransformed.set(FLT_MIN, FLT_MIN, FLT_MIN);

	// Iterate through all corners to find the bounding values
	for (int cCorner = 0; cCorner < 8; ++cCorner)
	{
		const Vector3& corner = corners[cCorner];

		// Find the max/min values for each axis
		// This could be in a seperate loop but I think this is significantly faster
		minTransformed[0] = std::min(corner[0], minTransformed[0]);
		minTransformed[1] = std::min(corner[1], minTransformed[1]);
		minTransformed[2] = std::min(corner[2], minTransformed[2]);

		maxTransformed[0] = std::min(corner[0], maxTransformed[0]);
		maxTransformed[1] = std::min(corner[1], maxTransformed[1]);
		maxTransformed[2] = std::min(corner[2], maxTransformed[2]);
	}
}

bool AABB::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	// Find the shortest intersection between the ray and an axis of the box
	for (int axis = 0; axis < 3; ++axis)
	{
		float cMin = (min[axis] - ray.origin[axis]) / ray.direction[axis];
		float cMax = (max[axis] - ray.origin[axis]) / ray.direction[axis];

		if (cMin > cMax) 
			std::swap(cMin, cMax);

		if ((tMin > cMax) || (tMax < cMin))
			return false;

		tMin = std::max(cMin, tMin);
		tMax = std::min(cMax, tMax);
	}

	hitInfo.distance = tMin;
	hitInfo.point = ray.origin + ray.direction * tMin;

	return true;
}