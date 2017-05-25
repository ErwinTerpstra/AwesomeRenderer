#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_

#include "awesomerenderer.h"
#include "component.h"
#include "treeelement.h"

#include "primitive.h"

namespace AwesomeRenderer
{
	class Material;

	class Renderable : public Component, public TreeElement
	{
	public:
		static const int id;

		Shape* shape;

		Material* material;

	public:
		Renderable();
		~Renderable();

		const Primitive& GetPrimitive() const { return shape->GetPrimitive(); }

		const Shape& GetShape() const { return *shape; }
	};

}

#endif