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

Vector3 BSDF::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material) const
{
	return specular->Sample(wo, wi, normal, hitInfo, material);

	Vector3 diffuseReflection(0.0f, 0.0f, 0.0f), specularReflection(0.0f, 0.0f, 0.0f);

	if (diffuse != NULL)
		diffuseReflection = diffuse->Sample(wo, wi, normal, hitInfo, material);
	
	if (specular != NULL)
		specularReflection = specular->Sample(wo, wi, normal, hitInfo, material);

	Vector3 fresnel = FresnelSchlick(VectorUtil<3>::Dot(normal, cml::normalize(wo + wi)), GetF0(material));
	return (diffuseReflection * (1.0f - fresnel)) + (specularReflection * fresnel);
}

void BSDF::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	specular->GenerateSampleVector(r, wo, normal, material, wi);
	return;

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
	return specular->CalculatePDF(wo, wi, normal, material);

	if (diffuse != NULL && specular != NULL)
		return 0.5f * (diffuse->CalculatePDF(wo, wi, normal, material) + specular->CalculatePDF(wo, wi, normal, material));
	else if (specular != NULL)
		return specular->CalculatePDF(wo, wi, normal, material);
	else if (diffuse != NULL)
		return diffuse->CalculatePDF(wo, wi, normal, material);

	return 0.0f;
}

// TODO: Generalize and remove
Vector3 BSDF::GetF0(const Material& material) const
{
	PbrMaterial* pbrMaterial = material.As<PbrMaterial>();

	if (pbrMaterial != NULL)
		return pbrMaterial->specular.subvector(3);

	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	if (phongMaterial != NULL)
		return phongMaterial->specularColor.subvector(3);

	return Vector3();

}
