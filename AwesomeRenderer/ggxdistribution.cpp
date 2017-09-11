#include "stdafx.h"
#include "ggxdistribution.h"
#include "microfacetmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

void GGXDistribution::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const MicrofacetMaterial& material, Vector3& wi) const
{
	float alpha2 = pow(material.roughness, 3);

	float phi = 2.0f * PI * r[1];

	float cosTheta = r[0] < 1.0f ? sqrt((1.0f - r[0]) / ((alpha2 - 1.0f) * r[0] + 1.0f)) : 0.0f;
	float theta = acos(cosTheta);

	Vector3 h;
	VectorUtil<3>::SphericalToCartesian(phi, theta, h);
	VectorUtil<3>::TransformSampleVector(normal, h, h);

	VectorUtil<3>::Reflect(wo, h, wi);
}

float GGXDistribution::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material) const
{
	Vector3 h = VectorUtil<3>::Normalize(wo + wi);

	float alpha2 = powf(material.roughness, 3);
	float NoH = Util::Clamp01(VectorUtil<3>::Dot(normal, h));
	float denom = (NoH * NoH * (alpha2 - 1.0f)) + 1.0f;

	return alpha2 / std::max((float)PI * denom * denom, 1e-7f);
}

float GGXDistribution::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material)
{
	float alpha2 = pow(material.roughness, 3);

	Vector3 h;
	if (!VectorUtil<3>::CalculateHalfVector(wo, wi, h))
		return 0.0;

	float cosTheta = VectorUtil<3>::Dot(normal, h);
	float sinTheta = sqrtf(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float denom = (cosTheta * cosTheta * (alpha2 - 1.0f)) + 1.0f;

	float pdf = (alpha2 / std::max((float)PI * denom * denom, 1e-7f));
	return pdf / (4 * VectorUtil<3>::Dot(wo, h));
}