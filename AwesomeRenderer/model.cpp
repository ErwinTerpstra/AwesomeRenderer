#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Model::Model() : meshes(), materials()
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
