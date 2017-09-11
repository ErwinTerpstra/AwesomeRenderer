#include "stdafx.h"
#include "microfacetspecular.h"
#include "microfacetdistribution.h"

#include "inputmanager.h"

#include "sampler.h"
#include "raycasthit.h"

#include "material.h"
#include "microfacetmaterial.h"

#include "ggxdistribution.h"
#include "blinndistribution.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MicrofacetSpecular::MicrofacetSpecular()
{
	normalDistribution = new BlinnDistribution();
}

MicrofacetSpecular::~MicrofacetSpecular()
{
	delete normalDistribution;
}

Vector3 MicrofacetSpecular::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material) const
{
	MicrofacetMaterial* microfacetMaterial = material.As<MicrofacetMaterial>();

	Color F0 = microfacetMaterial->specular;

	if (microfacetMaterial->specularMap != NULL)
		F0 *= microfacetMaterial->specularMap->Sample(hitInfo.uv);

	return SpecularCookTorrance(wo, normal, wi, F0.subvector(3), *microfacetMaterial);
}

void MicrofacetSpecular::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	const MicrofacetMaterial* pbrMaterial = material.As<MicrofacetMaterial>();
	normalDistribution->GenerateSampleVector(r, wo, normal, *pbrMaterial, wi);
}

float MicrofacetSpecular::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	const MicrofacetMaterial* pbrMaterial = material.As<MicrofacetMaterial>();
	return normalDistribution->CalculatePDF(wo, wi, normal, *pbrMaterial);
}

Vector3 MicrofacetSpecular::SpecularCookTorrance(const Vector3& wo, const Vector3& normal, const Vector3& wi, const Vector3& F0, const MicrofacetMaterial& material) const
{
	assert(VectorUtil<3>::IsNormalized(wo));
	assert(VectorUtil<3>::IsNormalized(normal));
	assert(VectorUtil<3>::IsNormalized(wi));

	float NoV = std::max(VectorUtil<3>::Dot(normal, wo), 0.0f);
	float NoL = std::max(VectorUtil<3>::Dot(normal, wi), 0.0f);

	if (NoV == 0.0f || NoL == 0.0f)
		return Vector3(0.0f, 0.0f, 0.0f);
	
	// Calculate the half vector
	Vector3 h = cml::normalize(wi + wo);

	Vector3 fresnel = FresnelSchlick(VectorUtil<3>::Dot(wo, h), F0);
	
	// Normal distribution
	float distribution = normalDistribution->Sample(wo, wi, normal, material);
	distribution = std::max(distribution, 0.0f);

	// Geometry term
	float alpha = material.roughness * material.roughness;
	float geometry;
	if (InputManager::Instance().GetKey('X'))
		geometry = GeometrySmith(wo, wi, normal, h, alpha);
	else if (InputManager::Instance().GetKey('C'))
		geometry = GeometryImplicit(wo, wi, normal, h);
	else
		geometry = GeometryGGX(wo, wi, normal, h, alpha);
	
	geometry = std::max(geometry, 0.0f);

	// Return the evaluated BRDF
	Vector3 result = (fresnel * geometry * distribution) / (4 * NoL * NoV);
	
	result[0] = std::max(result[0], 0.0f);
	result[1] = std::max(result[1], 0.0f);
	result[2] = std::max(result[2], 0.0f);

	return result;
}

float MicrofacetSpecular::GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h) const
{
	float NoV = Util::Clamp01(VectorUtil<3>::Dot(n, v));
	float NoL = Util::Clamp01(VectorUtil<3>::Dot(n, l));

	return NoL * NoV;
}

float MicrofacetSpecular::GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h) const
{
	float NoH = Util::Clamp01(VectorUtil<3>::Dot(n, h));
	float NoV = Util::Clamp01(VectorUtil<3>::Dot(n, v));
	float NoL = Util::Clamp01(VectorUtil<3>::Dot(n, l));
	float VoH = Util::Clamp(VectorUtil<3>::Dot(v, h), 1e-7f, 1.0f);

	return std::min(1.0f, std::min((2.0f * NoH * NoV) / VoH,
								   (2.0f * NoH * NoL) / VoH));
}

float MicrofacetSpecular::GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a) const
{
	a += 1;

	return G1Schlick(l, n, a) * G1Schlick(v, n, a);
}

float MicrofacetSpecular::G1Schlick(const Vector3& v, const Vector3& n, float a) const
{
	float NoV = Util::Clamp01(VectorUtil<3>::Dot(v, n));
	float k = (a * a) / 8;
	return NoV / (NoV * (1.0f - k) + k);
}

float MicrofacetSpecular::GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a) const
{
	return G1GGX(v, n, h, a) * G1GGX(l, n, h, a);
}

float MicrofacetSpecular::G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a) const
{
	float HoV = VectorUtil<3>::Dot(h, v);

	if (HoV <= 0.0f)
		return 0.0f;

	float a2 = a * a;

	//*
	float NoV = VectorUtil<3>::Dot(n, v);

	return (2.0f * NoV) / std::max(NoV + sqrt(a2 + (1.0f - a2) * NoV * NoV), 1e-7f);
	/*/

	float NoV2 = VectorUtil<3>::Dot(n, v);
	NoV2 = NoV2 * NoV2;

	float tan2 = (1.0f - NoV2) / NoV2;

	return (2.0f / std::max(1.0f + sqrt(1.0f + a2 * tan2), 1e-7f));
	//*/
}