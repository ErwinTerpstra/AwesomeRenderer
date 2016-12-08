#include "stdafx.h"
#include "awesomerenderer.h"
#include "meshex.h"
#include "mesh.h"
#include "triangle3d.h"
#include "kdtreenode.h"

using namespace AwesomeRenderer;

MeshEx::MeshEx(Mesh& mesh) : Extension(mesh), tree(10, 20), worldMtx(), world2object()
{

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

		// Create triangle object based on the vertex data
		Triangle3D* triangle = new Triangle3D(*this, vIdx0, vIdx1, vIdx2);
		triangles.push_back(triangle);
	}

	tree.rootNode->elements.insert(tree.rootNode->elements.end(), triangles.begin(), triangles.end());
}

MeshEx::~MeshEx()
{
	for (auto it = triangles.begin(); it != triangles.end(); ++it)
		delete *it;

	triangles.clear();
}

void MeshEx::OptimizeTree()
{
	// Optimize the tree with the local bounds, since it works with triangles in local coordinates
	Matrix44 mtx;
	mtx.identity();

	AABB localBounds = provider.bounds;
	localBounds.Transform(mtx);

	tree.Optimize(localBounds);

	//printf("[MeshEx]: Mesh tree optimized, analyzing...\n");
	//tree.Analyze();
}

void MeshEx::Transform(const Matrix44& mtx)
{
	worldMtx = mtx; 

	// Create an inversed transformation matrix to transform to object space
	world2object = worldMtx;
	world2object.inverse();
}

bool MeshEx::IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const
{
	// Transform ray to object space to use for intersection
	Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));

	// Transform maxDistance to object space
	Vector3 scale = cml::transform_vector(world2object, Vector3(1.0f, 1.0f, 1.0f));
	maxDistance *= std::max(scale[0], std::max(scale[1], scale[2]));

	// Perform intersection on the KD-tree
	if (tree.IntersectRay(objectSpaceRay, hitInfo, maxDistance))
	{
		hitInfo.point = cml::transform_point(worldMtx, hitInfo.point);
		hitInfo.normal = cml::transform_vector(worldMtx, hitInfo.normal);
		hitInfo.normal.normalize();

		// TODO: transform provided distance back to world space instead of recalculating?
		hitInfo.distance = (hitInfo.point - ray.origin).length();

		return true;
	}

	return false;
}

const Primitive& MeshEx::GetPrimitive() const
{
	return provider.bounds;
}