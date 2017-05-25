#ifndef _AREA_LIGHT_H_
#define _AREA_LIGHT_H_

#include "awesomerenderer.h"
#include "component.h"
#include "treeelement.h"

namespace AwesomeRenderer
{
	class Shape;
	class Primitive;
	class Material;

	class AreaLight : public Component, public TreeElement
	{
	public:
		static const int id;

		Primitive* primitive;
		Material* material;

	public:
		AreaLight();
		~AreaLight();

		const Primitive& GetPrimitive() const;
		const Shape& GetShape() const;
	};
}

#endif