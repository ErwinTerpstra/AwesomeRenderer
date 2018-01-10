
#include "awesomerenderer.h"
#include "model.h"
#include "mesh.h"

using namespace AwesomeRenderer;

Model::Model() : meshes(), materials(), bounds()
{

}


Model::~Model()
{
	meshes.clear();
	materials.clear();
}

void Model::AddMesh(Mesh* mesh, Material* material)
{
	meshes.push_back(mesh);
	materials.push_back(material);
}

void Model::CalculateBounds()
{
	Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 max(FLT_MIN, FLT_MIN, FLT_MIN);

	std::vector<Mesh*>::iterator it;

	// Combine all bounding volumes for the meshes
	for (it = meshes.begin(); it != meshes.end(); ++it)
	{
		const Mesh& mesh = **it;

		const Vector3& meshMin = mesh.bounds.Min();
		const Vector3& meshMax = mesh.bounds.Max();

		min[0] = std::min(meshMin[0], min[0]);
		min[1] = std::min(meshMin[1], min[1]);
		min[2] = std::min(meshMin[2], min[2]);

		max[0] = std::max(meshMax[0], max[0]);
		max[1] = std::max(meshMax[1], max[1]);
		max[2] = std::max(meshMax[2], max[2]);
	}

	bounds.Initialize(min, max);
}

void Model::TransformBounds(const Matrix44& mtx) 
{
	// Iterate through submeshes in a node
	for (uint32_t cMesh = 0; cMesh < meshes.size(); ++cMesh)
	{
		// Transform bounding shape of mesh according to world transformation
		Mesh* mesh = meshes[cMesh];
		mesh->bounds.Transform(mtx);
	}

	bounds.Transform(mtx);
}