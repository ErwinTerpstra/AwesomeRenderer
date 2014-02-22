#ifndef _MODEL_H_
#define _MODEL_H_

namespace AwesomeRenderer
{
	class Model : public Object, public Extendee<Model>
	{

	public:
		std::vector<Mesh*> meshes;

		std::vector<Material*> materials;

		AABB bounds;

	public:

		Model();
		~Model();

		void AddMesh(Mesh* mesh, Material* material);
		void CalculateBounds();

		virtual const Shape& GetShape() const { return bounds; }
	};

}

#endif