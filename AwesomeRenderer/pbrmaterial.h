#ifndef _PBR_MATERIAL_H_
#define _PBR_MATERIAL_H_

#include "awesomerenderer.h"
#include "material.h"

namespace AwesomeRenderer
{

	class PbrMaterial : public Extension<Material, PbrMaterial>
	{
	public:

		float roughness;

		float metallic;

		Color albedo;

		Color specular;

	public:
		PbrMaterial(Material& material);
		PbrMaterial::PbrMaterial(RayTracing::BSDF& bsdf, Material& material);
		~PbrMaterial();

		static uint32_t ExtensionID() { return Material::MATERIAL_PBR; }

	};

}


#endif