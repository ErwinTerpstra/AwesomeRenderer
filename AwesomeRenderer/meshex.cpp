#include "stdafx.h"
#include "awesomerenderer.h"
#include "meshex.h"
#include "mesh.h"
#include "triangle3d.h"

using namespace AwesomeRenderer;

MeshEx::MeshEx(Mesh& mesh) : Extension(mesh), tree(NULL), worldMtx(), world2object()
{

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

		// Create triangle object based on the vertex data
		Triangle3D* triangle;
		
		if (mesh.HasAttribute(Mesh::VERTEX_NORMAL))
			triangle = new Triangle3D(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2],
									  mesh.normals[vIdx0], mesh.normals[vIdx1], mesh.normals[vIdx2]);
		else
			triangle = new Triangle3D(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);

		triangles.push_back(triangle);
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

	// Create an inversed transformation matrix to transform to object space
	world2object = worldMtx;
	world2object.inverse();
}

bool MeshEx::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	//return provider.bounds.IntersectRay(ray, hitInfo);

	// Transform ray to object space to use for intersection
	Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));
	objectSpaceRay.direction.normalize();

	// Perform intersection on the KD-tree
	if (tree.IntersectRay(objectSpaceRay, hitInfo))
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

const Primitive& MeshEx::GetShape() const
{
	return provider.bounds;
}