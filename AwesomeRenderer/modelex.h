#ifndef _MODEL_EX_H_
#define _MODEL_EX_H_

#include "awesomerenderer.h"

#include "model.h"

namespace AwesomeRenderer
{
	class MeshEx;

	class ModelEx : public Extension<Model, ModelEx>
	{
	public:

		std::vector<MeshEx*> meshes;

	public:
		ModelEx(Model& model);
		~ModelEx();

		static uint32_t ExtensionID() { return Model::MODEL_EX; }

	};
}

#endif