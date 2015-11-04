#ifndef _PHONG_MATERIAL_H_
#define _PHONG_MATERIAL_H_

#include "awesomerenderer.h"
#include "material.h"

namespace AwesomeRenderer
{

	class PhongMaterial : public Material
	{
	public:

		Sampler* diffuseMap;
		Sampler* specularMap;
		Sampler* normalMap;

		Color diffuseColor;
		Color specularColor;

		float shininess;

	public:
		PhongMaterial();
		~PhongMaterial();
	};

}


#endif