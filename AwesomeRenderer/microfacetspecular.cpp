#include "stdafx.h"
#include "microfacetspecular.h"

#include "inputmanager.h"

#include "material.h"
#include "pbrmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MicrofacetSpecular::MicrofacetSpecular()
{

}

Vector3 MicrofacetSpecular::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material)
{
	PbrMaterial* pbrMaterial = material.As<PbrMaterial>();
	Vector3 F0 = pbrMaterial->specular.subvector(3);
	
	Vector3 radiance(0.0f, 0.0f, 0.0f);
	
	Vector3 ks;
	radiance += SpecularCookTorrance(wo, normal, wi, F0, pbrMaterial->roughness, ks);

	// TODO: Split diffuse to other BSDF class
	Vector3 kd = (1.0f - ks) * (1.0f - pbrMaterial->metallic);
	radiance += DiffuseLambert(pbrMaterial->albedo.subvector(3)) * kd;

	return radiance;
}

Vector3 MicrofacetSpecular::DiffuseLambert(const Vector3& albedo)
{
	return albedo / PI;
}

Vector3 MicrofacetSpecular::SpecularCookTorrance(const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks)
{
	assert(VectorUtil<3>::IsNormalized(v));
	assert(VectorUtil<3>::IsNormalized(n));
	assert(VectorUtil<3>::IsNormalized(l));

	roughness = roughness * roughness;

	// Calculate the half vector
	Vector3 h = cml::normalize(l + v);

	// Fresnel term
	Vector3 fresnel = InputManager::Instance().GetKey('Z') ? F0 : FresnelSchlick(Util::Clamp01(cml::dot(h, l)), F0);
	ks = fresnel;

	float distribution, geometry;

	// Normal distribution & geometry term
	if (InputManager::Instance().GetKey('X'))
	{
		distribution = DistributionBlinn(n, h, RoughnessToShininess(roughness));
		geometry = GeometrySmith(v, l, n, h, roughness);
	}
	else
	{
		distribution = DistributionGGX(n, h, roughness);
		geometry = GeometryGGX(v, l, n, h, roughness);
	}

	if (InputManager::Instance().GetKey('C'))
		geometry = GeometryImplicit(v, l, n, h);

	distribution = std::max(distribution, 0.0f);
	geometry = std::max(geometry, 0.0f);

	// Calculate the Cook-Torrance denominator
	float denominator = std::max(4 * Util::Clamp01(cml::dot(n, v)) * Util::Clamp01(cml::dot(n, l)), 1e-7f);

	// Return the evaluated BRDF
	Vector3 result = ((fresnel * geometry * distribution) / denominator);
	
	result[0] = Util::Clamp01(result[0]);
	result[1] = Util::Clamp01(result[1]);
	result[2] = Util::Clamp01(result[2]);

	//result[0] = std::max(result[0], 0.0f);
	//result[1] = std::max(result[1], 0.0f);
	//result[2] = std::max(result[2], 0.0f);

	return result;
}

float MicrofacetSpecular::RoughnessToShininess(float a)
{
	return std::max((2.0f / (a * a)) - 2.0f, 1e-7f);
}

float MicrofacetSpecular::DistributionBlinn(const Vector3 & n, const Vector3& h, float e)
{
	float NoH = Util::Clamp01(cml::dot(n, h));

	return ((e + 2) * INV_TWO_PI) * std::pow(NoH, e);
}

float MicrofacetSpecular::DistributionGGX(const Vector3& n, const Vector3& h, float alpha)
{
	float alpha2 = alpha * alpha;
	float NoH = Util::Clamp01(cml::dot(n, h));
	float denom = (NoH * NoH * (alpha2 - 1.0f)) + 1.0f;
	return alpha2 / std::max((float)PI * denom * denom, 1e-7f);
}

float MicrofacetSpecular::GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h)
{
	float NoV = Util::Clamp01(cml::dot(n, v));
	float NoL = Util::Clamp01(cml::dot(n, l));

	return NoL * NoV;
}

float MicrofacetSpecular::GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h)
{
	float NoH = Util::Clamp01(cml::dot(n, h));
	float NoV = Util::Clamp01(cml::dot(n, v));
	float NoL = Util::Clamp01(cml::dot(n, l));
	float VoH = Util::Clamp(cml::dot(v, h), 1e-7f, 1.0f);

	return std::min(1.0f, std::min((2.0f * NoH * NoV) / VoH,
								   (2.0f * NoH * NoL) / VoH));
}

float MicrofacetSpecular::GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a)
{
	a += 1;

	return G1Schlick(l, n, a) * G1Schlick(v, n, a);
}

float MicrofacetSpecular::G1Schlick(const Vector3& v, const Vector3& n, float a)
{
	float NoV = Util::Clamp01(cml::dot(v, n));
	float k = (a * a) / 8;
	return NoV / (NoV * (1.0f - k) + k);
}

float MicrofacetSpecular::GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a)
{
	return G1GGX(v, n, h, a) * G1GGX(l, n, h, a);
}

float MicrofacetSpecular::G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a)
{
	float HoV = cml::dot(h, v);

	if (HoV <= 0.0f)
		return 0.0f;

	float a2 = a * a;

	//*
	float NoV = cml::dot(n, v);

	return (2.0f * NoV) / std::max(NoV + sqrt(a2 + (1.0f - a2) * NoV * NoV), 1e-7f);
	/*/

	float NoV2 = cml::dot(n, v);
	NoV2 = NoV2 * NoV2;

	float tan2 = (1.0f - NoV2) / NoV2;

	return (2.0f / std::max(1.0f + sqrt(1.0f + a2 * tan2), 1e-7f));
	//*/
}

Vector3 MicrofacetSpecular::FresnelSchlick(float cosT, Vector3 F0)
{
	return F0 + (1.0f - F0) * pow(1 - cosT, 5);
}
