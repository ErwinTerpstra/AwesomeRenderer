#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_

#include "awesomerenderer.h"
#include "primitive.h"
#include "shape.h"
#include "component.h"

namespace AwesomeRenderer
{
	class Shape;
	class Material;

	class Renderable : public Component
	{
	public:
		static const int id;

		Shape* shape;

		Material* material;

	public:
		Renderable();
		~Renderable();
	};

}

#endif