
#include "blinndistribution.h"
#include "microfacetmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

void BlinnDistribution::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const MicrofacetMaterial& material, Vector3& wi) const
{
	float e = RoughnessToShininess(material.roughness);

	float phi = 2.0f * PI * r[1];
	float theta = acosf(powf(r[0], 1.0f / (e + 1.0f)));

	Vector3 h;
	VectorUtil<3>::SphericalToCartesian(phi, theta, h);
	VectorUtil<3>::TransformSampleVector(normal, h, h);

	VectorUtil<3>::Reflect(wo, h, wi);
}

float BlinnDistribution::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material) const
{
	float e = RoughnessToShininess(material.roughness);
	Vector3 h = VectorUtil<3>::Normalize(wo + wi);

	float NoH = Util::Clamp01(VectorUtil<3>::Dot(normal, h));

	return ((e + 2) * INV_TWO_PI) * std::pow(NoH, e);
}

float BlinnDistribution::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material)
{
	float e = RoughnessToShininess(material.roughness);
	Vector3 h = VectorUtil<3>::Normalize(wo + wi);

	float NoH = Util::Clamp01(VectorUtil<3>::Dot(normal, h));
	float VoH = VectorUtil<3>::Dot(wo, h);

	if (VoH <= 0.0f)
		return 0.0;

	float pdf = (e + 1.0f) * std::pow(NoH, e) * INV_TWO_PI;
	return pdf / (4 * VoH);
}