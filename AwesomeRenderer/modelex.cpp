#include "stdafx.h"
#include "awesomerenderer.h"
#include "modelex.h"

#include "model.h"

#include "mesh.h"
#include "meshex.h"

using namespace AwesomeRenderer;

ModelEx::ModelEx(Model& model) : Extension(model)
{
	std::vector<Mesh*>::const_iterator it;

	for (it = model.meshes.begin(); it != model.meshes.end(); ++it)
	{
		MeshEx* mesh = new MeshEx(**it);
		mesh->OptimizeTree();
		meshes.push_back(mesh);
	}
}

ModelEx::~ModelEx()
{
	std::vector<MeshEx*>::iterator it;

	for (it = meshes.begin(); it != meshes.end(); ++it)
	{
		delete (*it);
	}

	meshes.clear();
}