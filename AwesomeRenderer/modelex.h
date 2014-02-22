#ifndef _MODEL_EX_H_
#define _MODEL_EX_H_

namespace AwesomeRenderer
{
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