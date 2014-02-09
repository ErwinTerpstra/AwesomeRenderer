#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Model::Model() : meshes(), materials()
{

}


Model::~Model()
{

	{
		//std::vector<Mesh*>::iterator it;

		//for (it = meshes.begin(); it != meshes.end(); ++it)
			//delete (*it);

		meshes.clear();
	}

	{
		//std::vector<Material*>::iterator it;

		//for (it = materials.begin(); it != materials.end(); ++it)
			//delete (*it);

		materials.clear();
	}
}

Mesh* Model::AddMesh(Mesh::VertexAttributes attributes, const Material& material)
{
	Mesh mesh(attributes);

	meshes.push_back(mesh);
	materials.push_back(material);

	return &meshes[meshes.size() - 1];
}
