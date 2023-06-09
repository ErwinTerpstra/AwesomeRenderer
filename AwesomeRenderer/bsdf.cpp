
#include "bsdf.h"
#include "bxdf.h"

#include "microfacetmaterial.h"
#include "phongmaterial.h"
#include "sampler.h"
#include "texture.h"

#include "rendercontext.h"
#include "rendertarget.h"
#include "raycasthit.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BSDF::BSDF() : diffuse(NULL), specular(NULL)
{

}

BSDF::BSDF(BxDF* diffuse, BxDF* specular) : diffuse(diffuse), specular(specular)
{

}

Vector3 BSDF::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext, BxDFTypes typeMask) const
{
	Vector3 diffuseReflection(0.0f, 0.0f, 0.0f), specularReflection(0.0f, 0.0f, 0.0f);

	if (diffuse != NULL && (typeMask & BXDF_DIFFUSE) != 0)
		diffuseReflection = diffuse->Sample(wo, wi, normal, hitInfo, material, renderContext);
	
	if (specular != NULL && (typeMask & BXDF_SPECULAR) != 0)
	{
		MicrofacetMaterial* microfacetMaterial = material.As<MicrofacetMaterial>();

		if (microfacetMaterial != NULL)
		{
			Color F0 = microfacetMaterial->specular;

			if (microfacetMaterial->specularMap != NULL)
				F0 *= microfacetMaterial->specularMap->SampleMipMaps(hitInfo.uv, hitInfo.distance, hitInfo.surfaceAreaToTextureRatio, renderContext.renderTarget->frameBuffer->GetResolution());

			Vector3 h = VectorUtil<3>::Normalize(wo + wi);
			Vector3 fresnel = Vector3(1.0f, 1.0f, 1.0f) - RenderUtil::FresnelSchlick(VectorUtil<3>::Dot(wo, h), F0.subvector(3));

			diffuseReflection = diffuseReflection * fresnel * (1.0f - microfacetMaterial->metallic);
		}

		specularReflection = specular->Sample(wo, wi, normal, hitInfo, material, renderContext);
	}

	return diffuseReflection + specularReflection;
}

void BSDF::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	// Select the BSDF to generate a sample vector from
	if (diffuse != NULL && specular != NULL)
	{
		Vector2 rl = r;

		if (rl[0] < 0.5f)
		{
			rl[0] = 2.0f * rl[0];

			diffuse->GenerateSampleVector(rl, wo, normal, material, wi);
		}
		else
		{
			rl[0] = 2.0f * (rl[0] - 0.5f);

			specular->GenerateSampleVector(rl, wo, normal, material, wi);
		}
	}
	else if (specular != NULL)
		specular->GenerateSampleVector(r, wo, normal, material, wi);
	else if (diffuse != NULL)
		diffuse->GenerateSampleVector(r, wo, normal, material, wi);

	// Make sure the sample vector is in the same hemisphere as the normal
	if (VectorUtil<3>::Dot(normal, wi) < 0.0f)
	{
		// Calculate the perfect specular direction
		Vector3 wr;
		VectorUtil<3>::Reflect(wo, normal, wr);

		// Reflect the sample direction around the perfect specular direction to make sure it is in the upper hemisphere
		VectorUtil<3>::Reflect(wi, wr, wi);
	}
}

float BSDF::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	if (diffuse != NULL && specular != NULL)
		return 0.5f * (diffuse->CalculatePDF(wo, wi, normal, material) + specular->CalculatePDF(wo, wi, normal, material));
	else if (specular != NULL)
		return specular->CalculatePDF(wo, wi, normal, material);
	else if (diffuse != NULL)
		return diffuse->CalculatePDF(wo, wi, normal, material);

	return 0.0f;
}
