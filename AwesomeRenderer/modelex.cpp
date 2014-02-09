#include "awesomerenderer.h"

using namespace AwesomeRenderer;

ModelEx::ModelEx(const Model& model)
{

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