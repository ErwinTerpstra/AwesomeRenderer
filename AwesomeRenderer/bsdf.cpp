#include "stdafx.h"
#include "bsdf.h"
#include "bxdf.h"

#include "pbrmaterial.h"
#include "phongmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BSDF::BSDF() : diffuse(NULL), specular(NULL)
{

}

BSDF::BSDF(BxDF* diffuse, BxDF* specular) : diffuse(diffuse), specular(specular)
{

}

Vector3 BSDF::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	Vector3 diffuseRadiance(0.0f, 0.0f, 0.0f), specularRadiance(0.0f, 0.0f, 0.0f);

	if (diffuse != NULL)
		diffuseRadiance = diffuse->Sample(wo, wi, normal, material);
	
	if (specular != NULL)
		specularRadiance = specular->Sample(wo, wi, normal, material);

	return SpecularTradeoff(diffuseRadiance, specularRadiance, normal, cml::normalize(wo + wi), material);
}

Vector3 BSDF::SpecularTradeoff(const Vector3& diffuseRadiance, const Vector3& specularRadiance, const Vector3& n, const Vector3& v, const Material& material) const
{
	Vector3 fresnel = FresnelSchlick(VectorUtil<3>::Dot(n, v), GetF0(material));
	return (diffuseRadiance * (1.0f - fresnel)) + (specularRadiance * fresnel);
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
