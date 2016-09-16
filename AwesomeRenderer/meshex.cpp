#include "stdafx.h"
#include "awesomerenderer.h"
#include "meshex.h"
#include "mesh.h"
#include "triangle3d.h"

using namespace AwesomeRenderer;

MeshEx::MeshEx(Mesh& mesh) : Extension(mesh), tree(NULL), worldMtx()
{

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

		// Create triangle object based on the vertex data
		Triangle3D& triangle = AddTri(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);
	}

	tree.objects.insert(tree.objects.end(), triangles.begin(), triangles.end());
}

MeshEx::~MeshEx()
{
	for (auto it = triangles.begin(); it != triangles.end(); ++it)
		delete *it;

	triangles.clear();
}

Triangle3D& MeshEx::AddTri(const Vector3& a, const Vector3& b, const Vector3& c)
{
	Triangle3D* triangle = new Triangle3D(a, b, c);
	triangles.push_back(triangle);

	return *triangle;
}

void MeshEx::OptimizeTree()
{
	tree.Optimize(provider.bounds);
}

void MeshEx::Transform(const Matrix44& mtx)
{
	worldMtx = mtx;
}

bool MeshEx::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	// Check if some part of the mesh is hit by the ray
	RaycastHit boundsHitInfo;
	if (!provider.bounds.IntersectRay(ray, boundsHitInfo))
		return false;

	// Create an inversed transformation matrix to transform to object space
	Matrix44 world2object = worldMtx;
	world2object.inverse();

	// Transform ray to object space to use for intersection
	Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));
	objectSpaceRay.direction.normalize();

	// Perform intersection on the KD-tree
	return tree.IntersectRay(ray, hitInfo);
}

const Primitive& MeshEx::GetShape() const
{
	return provider.bounds;
}