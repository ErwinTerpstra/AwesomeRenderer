#ifndef _MODEL_H_
#define _MODEL_H_

namespace AwesomeRenderer
{

	class Model
	{

	public:
		std::vector<Mesh*> meshes;

		std::vector<Material*> materials;
	public:

		Model();
		~Model();

		void AddMesh(Mesh* mesh, Material* material);
	};

}

#endif