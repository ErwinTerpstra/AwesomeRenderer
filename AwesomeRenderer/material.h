#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Shader;
	class Sampler;

	class Material : public ExtensionProvider<Material>
	{
	public:
		enum Extensions
		{
			MATERIAL_PHONG,
			MATERIAL_PBR
		};

		bool translucent;

		Shader* shader;

	public:
		Material();
		virtual ~Material() { };
	};

}


#endif