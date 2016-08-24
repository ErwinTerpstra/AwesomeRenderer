#ifndef _MODEL_H_
#define _MODEL_H_

#include "awesomerenderer.h"
#include "shape.h"
#include "aabb.h"
#include "component.h"

namespace AwesomeRenderer
{
	class Mesh;
	class Material;

	class Model : public Component, public ExtensionProvider<Model>
	{

	public:
		enum Extensions
		{
			MODEL_EX
		};

		static const int id;

		std::vector<Mesh*> meshes;

		std::vector<Material*> materials;

		AABB bounds;

	public:

		Model();
		~Model();

		void AddMesh(Mesh* mesh, Material* material);
		void CalculateBounds();

		void TransformBounds(const Matrix44& mtx);
	};

}

#endif