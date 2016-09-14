#include "stdafx.h"
#include "montecarlointegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"

#include "shadinginfo.h"
#include "random.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MonteCarloIntegrator::MonteCarloIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer), sampleCount(16), random(Random::instance)
{

}

Vector3 MonteCarloIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, int depth)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);

	PbrMaterial* pbrMaterial = material.As<PbrMaterial>();

	for (int sampleIdx = 0; sampleIdx < sampleCount; ++sampleIdx)
	{
		float pdf;
		Vector3 reflectionDirection = GenerateSampleVector(-ray.direction, hitInfo.normal, pbrMaterial->roughness, pdf);

		if (pdf < 1e-5f)
			continue;

		// Reflection
		Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-5f, reflectionDirection);

		ShadingInfo reflectionShading;
		rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

		float NoL = cml::dot(hitInfo.normal, reflectionDirection);
		assert(NoL + 1e-5f >= 0.0f && NoL - 1e-5f <= 1.0f);

		Vector3 lightRadiance = reflectionShading.color.subvector(3);

		if (lightRadiance.length_squared() <= 1e-5)
			continue;

		radiance += material.bsdf->Sample(-ray.direction, reflectionDirection, hitInfo.normal, material) * lightRadiance * NoL / pdf;
	}
	
	return radiance / sampleCount;
}

Vector3 MonteCarloIntegrator::GenerateSampleVector(const Vector3& v, const Vector3& n, float roughness, float& pdf)
{
	float r1 = random.NextFloat();
	float r2 = random.NextFloat();

	float phi, theta;
	ImportanceSampleGGX(Vector2(r1, r2), roughness, phi, theta);
	pdf = PDFGGX(phi, theta, roughness);

	float sinTheta = sinf(theta);
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);

	Vector3 sample = Vector3(x, cosf(theta), z);

	assert(VectorUtil<3>::IsNormalized(sample));

	Vector3 right, forward;
	VectorUtil<3>::OrthoNormalize(n, right, forward);

	assert(fabs(cml::dot(n, forward)) < 1e-5f);
	assert(fabs(cml::dot(n, right)) < 1e-5f);
	assert(fabs(cml::dot(right, forward)) < 1e-5f);

	Matrix33 transform(
		right[0], right[1], right[2],
		n[0], n[1], n[2],
		forward[0], forward[1], forward[2]
		);

	assert(cml::dot(sample, Vector3(0.0f, 1.0f, 0.0f)) >= 0.0f - 1e-5f);

	sample = transform_vector(transform, sample);

	assert(VectorUtil<3>::IsNormalized(sample));

	return sample;
}

void MonteCarloIntegrator::ImportanceSampleGGX(const Vector2& r, float alpha, float& phi, float& theta)
{
	phi = 2.0f * PI * r[0];
	theta = acos(sqrt((1.0f - r[1]) / ((alpha * alpha - 1.0f) * r[1] + 1.0f)));
}

float MonteCarloIntegrator::PDFGGX(float phi, float theta, float alpha)
{
	float alpha2 = alpha * alpha;
	float cosTheta = cosf(theta);
	float denom = (cosTheta * cosTheta * (alpha2 - 1.0f)) + 1.0f;

	return (alpha2 / std::max((float)PI * denom * denom, 1e-7f)) * cosTheta * sinf(theta);
}