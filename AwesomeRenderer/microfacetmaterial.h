#ifndef _MICROFACET_MATERIAL_H_
#define _MICROFACET_MATERIAL_H_

#include "awesomerenderer.h"
#include "material.h"

namespace AwesomeRenderer
{

	class MicrofacetMaterial : public Extension<Material, MicrofacetMaterial>
	{
	public:
		static RayTracing::BSDF metallicBSDF;
		static RayTracing::BSDF dielectricBSDF;

		float roughness;

		float metallic;

		Color albedo;

		Color specular;

		Sampler* albedoMap;
		Sampler* specularMap;
		Sampler* normalMap;

	public:
		MicrofacetMaterial(Material& material);
		MicrofacetMaterial(RayTracing::BSDF& bsdf, Material& material);

		~MicrofacetMaterial();

		static uint32_t ExtensionID() { return Material::MATERIAL_PBR; }

	};

}


#endif