#include "stdafx.h"
#include "bsdf.h"
#include "bxdf.h"

#include "pbrmaterial.h"
#include "phongmaterial.h"

#include "raycasthit.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BSDF::BSDF() : diffuse(NULL), specular(NULL)
{

}

BSDF::BSDF(BxDF* diffuse, BxDF* specular) : diffuse(diffuse), specular(specular)
{

}

Vector3 BSDF::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, BxDFTypes typeMask) const
{
	Vector3 diffuseReflection(0.0f, 0.0f, 0.0f), specularReflection(0.0f, 0.0f, 0.0f);

	if (diffuse != NULL && (typeMask & BXDF_DIFFUSE) != 0)
		diffuseReflection = diffuse->Sample(wo, wi, normal, hitInfo, material);
	
	if (specular != NULL && (typeMask & BXDF_SPECULAR) != 0)
		specularReflection = specular->Sample(wo, wi, normal, hitInfo, material);

	return (diffuseReflection * (1.0 - specularReflection)) + specularReflection;
}

void BSDF::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
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
