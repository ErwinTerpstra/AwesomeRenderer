#include "stdafx.h"
#include "awesomerenderer.h"
#include "meshex.h"
#include "mesh.h"
#include "meshtriangle.h"
#include "kdtreenode.h"

using namespace AwesomeRenderer;

template class KDTree<MeshTriangle>;

MeshEx::MeshEx(Mesh& mesh) : Extension(mesh), tree(20), worldMtx(), world2object()
{
	triangles.reserve(mesh.indices.size() / 3);

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex];
		int vIdx1 = mesh.indices[cIndex + 1];
		int vIdx2 = mesh.indices[cIndex + 2];

		// Retrieve vertex positions for this triangle
		Vector3 v0 = mesh.vertices[vIdx0];
		Vector3 v1 = mesh.vertices[vIdx1];
		Vector3 v2 = mesh.vertices[vIdx2];

		// Create triangle object based on the vertex data
		MeshTriangle triangle(v0, v1, v2, cIndex / 3);
		
		if (triangle.IsLine())
			continue;

		triangles.push_back(triangle);
		
		// TODO: These references break if the vector decides to resize. Use simple array?
		tree.elements.push_back(&triangles.back());
	}

	//tree.elements.insert(tree.elements.end(), triangles.begin(), triangles.end());
}

MeshEx::~MeshEx()
{
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

	printf("[MeshEx]: Mesh tree optimized, analyzing...\n");
	tree.Analyze();
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
		// Interpolate vertex attributes of the hit triangle
		const MeshTriangle* tri = dynamic_cast<const MeshTriangle*>(hitInfo.element);

		// Retrieve the vertex indices of this triangle
		int vIdx0 = provider.indices[tri->faceIdx * 3];
		int vIdx1 = provider.indices[tri->faceIdx * 3 + 1];
		int vIdx2 = provider.indices[tri->faceIdx * 3 + 2];

		if (provider.HasAttribute(Mesh::VERTEX_NORMAL))
		{
			VectorUtil<3>::Interpolate(
				provider.normals[vIdx0],
				provider.normals[vIdx1],
				provider.normals[vIdx2],
				hitInfo.barycentricCoords, hitInfo.normal);
		}
		else
			hitInfo.normal = tri->normal;

		if (provider.HasAttribute(Mesh::VERTEX_TEXCOORD))
		{
			Vector2 uv0 = provider.texcoords[vIdx0];
			Vector2 uv1 = provider.texcoords[vIdx1];
			Vector2 uv2 = provider.texcoords[vIdx2];

			VectorUtil<2>::Interpolate(uv0, uv1, uv2, hitInfo.barycentricCoords, hitInfo.uv);

			float surfaceArea = Triangle<Vector3>::Area(cml::transform_point(worldMtx, tri->v[0]),
														cml::transform_point(worldMtx, tri->v[1]),
														cml::transform_point(worldMtx, tri->v[2]));
			float texels = Triangle<Vector2>::Area(uv0, uv1, uv2);
			
			assert(surfaceArea > 0.0f);

			if (texels > 0.0f)
				hitInfo.surfaceAreaToTextureRatio = surfaceArea / texels;
		}

		hitInfo.point = cml::transform_point(worldMtx, hitInfo.point);
		hitInfo.normal = cml::transform_vector(worldMtx, hitInfo.normal);
		hitInfo.normal.normalize();
		
		// TODO: transform provided distance back to world space instead of recalculating?
		hitInfo.distance = (hitInfo.point - ray.origin).length();
		
		return true;
	}

	return false;
}
