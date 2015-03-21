#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_

#include "awesomerenderer.h"
#include "primitive.h"
#include "object.h"
#include "component.h"

namespace AwesomeRenderer
{
	class Primitive;
	class Material;

	class Renderable : public Component, public Object
	{
	public:
		static const int id;

		Primitive* primitive;

		Material* material;

	public:
		Renderable();
		~Renderable();

		const Primitive& GetShape() const;

	};

}

#endif