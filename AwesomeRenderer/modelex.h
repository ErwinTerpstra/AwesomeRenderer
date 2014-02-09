#ifndef _MODEL_EX_H_
#define _MODEL_EX_H_

namespace AwesomeRenderer
{
	class ModelEx
	{
	public:

		std::vector<MeshEx*> meshes;

	public:
		ModelEx(const Model& model);
		~ModelEx();

	};
}

#endif