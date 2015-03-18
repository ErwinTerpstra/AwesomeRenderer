#ifndef _MODEL_EX_H_
#define _MODEL_EX_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Model;

	class MeshEx;

	class ModelEx : public Extension<Model>
	{
	public:

		std::vector<MeshEx*> meshes;

	public:
		ModelEx(Model& model);
		~ModelEx();

	};
}

#endif