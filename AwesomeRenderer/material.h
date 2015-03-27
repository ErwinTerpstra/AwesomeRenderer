#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Shader;
	class Sampler;

	class Material
	{
	public:
		bool translucent;

		float shininess;

		Sampler* diffuseMap;
		Sampler* specularMap;
		Sampler* normalMap;

		Color diffuseColor;
		Color specularColor;

		Shader* shader;

	public:
		Material();

	};


}


#endif