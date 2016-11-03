#include "stdafx.h"
#include "bsdf.h"
#include "bxdf.h"

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
	Vector3 diffuseRadiance, specularRadiance;
	
	if (diffuse != NULL)
		diffuseRadiance = diffuse->Sample(wo, wi, normal, material);
	
	if (specular != NULL)
		specularRadiance = specular->Sample(wo, wi, normal, material);

	return ConserveEnergy(diffuseRadiance, specularRadiance);
}


Vector3 BSDF::ConserveEnergy(const Vector3& diffuseRadiance, const Vector3& specularRadiance) const
{
	return diffuseRadiance * (1.0f - specularRadiance) + specularRadiance;
}