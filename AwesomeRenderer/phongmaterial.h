#ifndef _PHONG_MATERIAL_H_
#define _PHONG_MATERIAL_H_

#include "awesomerenderer.h"
#include "material.h"

namespace AwesomeRenderer
{

	class PhongMaterial : public Extension<Material, PhongMaterial>
	{
	public:
		static RayTracing::BSDF bsdf;

		Sampler* diffuseMap;
		Sampler* specularMap;
		Sampler* normalMap;

		Color diffuseColor;
		Color specularColor;

		float shininess;

	public:
		PhongMaterial(Material& material);
		~PhongMaterial();

		static uint32_t ExtensionID() { return Material::MATERIAL_PHONG; }
	};

}


#endif