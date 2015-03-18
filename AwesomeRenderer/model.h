#ifndef _MODEL_H_
#define _MODEL_H_

#include "awesomerenderer.h"
#include "object.h"
#include "aabb.h"
#include "component.h"

namespace AwesomeRenderer
{
	class Mesh;
	class Material;

	class Model : public Object, public Component, public Extendee<Model>
	{

	public:
		static const int ID;

		std::vector<Mesh*> meshes;

		std::vector<Material*> materials;

		AABB bounds;

	public:

		Model();
		~Model();

		void AddMesh(Mesh* mesh, Material* material);
		void CalculateBounds();

		virtual const Primitive& GetShape() const { return bounds; }
	};

}

#endif