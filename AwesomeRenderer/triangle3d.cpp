#include "stdafx.h"
#include "awesomerenderer.h"

#include "triangle3d.h"

#include "plane.h"
#include "aabb.h"

#include "meshex.h"

using namespace AwesomeRenderer;

Triangle3D::Triangle3D(const MeshEx& mesh, uint32_t vIdx0, uint32_t vIdx1, uint32_t vIdx2) :
	Triangle(mesh.provider.vertices[vIdx0], mesh.provider.vertices[vIdx1], mesh.provider.vertices[vIdx2]), Primitive(), mesh(mesh)
{
	vIdx[0] = vIdx0;
	vIdx[1] = vIdx1;
	vIdx[2] = vIdx2;

	CalculateNormal();
	PreCalculateBarycentric();
}


Triangle3D::Triangle3D(const Triangle3D& other) :
	Triangle3D(other.mesh, other.vIdx[0], other.vIdx[1], other.vIdx[2])
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
	/*
	v[0] = cml::transform_point(mtx, vO[0]);
	v[1] = cml::transform_point(mtx, vO[1]);
	v[2] = cml::transform_point(mtx, vO[2]);

	CalculateNormal();
	PreCalculateBarycentric();
	*/

	// TODO: Find a way to implement this without having to store additional data, which makes the path tracer slow
	// Or make it possible to inherit from Shape without having to implement this method
	assert(false && "Not implemented.");
}

bool Triangle3D::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
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

	if (mesh.provider.HasAttribute(Mesh::VERTEX_NORMAL))
	{
		VectorUtil<3>::Interpolate(
			mesh.provider.normals[vIdx[0]],
			mesh.provider.normals[vIdx[1]],
			mesh.provider.normals[vIdx[2]],
			hitInfo.barycentricCoords, hitInfo.normal);
	}
	else
		hitInfo.normal = normal;

	if (mesh.provider.HasAttribute(Mesh::VERTEX_TEXCOORD))
	{
		VectorUtil<2>::Interpolate(
			mesh.provider.texcoords[vIdx[0]],
			mesh.provider.texcoords[vIdx[1]],
			mesh.provider.texcoords[vIdx[2]],
			hitInfo.barycentricCoords, hitInfo.uv);
	}		
		
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