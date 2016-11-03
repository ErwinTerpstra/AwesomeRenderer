#include "stdafx.h"
#include "montecarlointegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"

#include "bxdf.h"
#include "bsdf.h"

#include "shadinginfo.h"
#include "random.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MonteCarloIntegrator::MonteCarloIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer), sampleCount(16), random(Random::instance)
{

}

Vector3 MonteCarloIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 radiance = material.emission.subvector(3);
	
	radiance += SampleDirectLight(ray, hitInfo, material, context);

	if (depth < rayTracer.maxDepth)
	{
		Vector3 diffuse = Integrate(ray, hitInfo, *material.bsdf->diffuse, material, sampleCount, depth);
		Vector3 specular = Vector3(0.0f, 0.0f, 0.0f);// Integrate(ray, hitInfo, *material.bsdf->specular, material, sampleCount, depth);

		radiance += material.bsdf->ConserveEnergy(diffuse, specular);
	}
	
	return radiance;
}

Vector3 MonteCarloIntegrator::Integrate(const Ray& ray, const RaycastHit& hitInfo, const BxDF& bxdf, const Material& material, uint32_t samples, int depth)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);
	
	for (uint32_t sampleIdx = 0; sampleIdx < samples; ++sampleIdx)
	{
		float pdf;
		Vector3 reflectionDirection = GenerateSampleVector(ray.direction, hitInfo.normal, bxdf, material, pdf);
		
		// TODO: Is it correct to skip a sample when it's PDF is zero?
		if (pdf < 1e-5f)
			continue;

		// Calculate incoming light along this sample vector
		Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-3f, reflectionDirection);

		ShadingInfo reflectionShading;
		rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

		float NoL = cml::dot(hitInfo.normal, reflectionDirection);
		assert(NoL + 1e-5f >= 0.0f && NoL - 1e-5f <= 1.0f);

		Vector3 lightRadiance = reflectionShading.color.subvector(3);

		if (lightRadiance.length_squared() <= 1e-5)
			continue;

		radiance += bxdf.Sample(-ray.direction, reflectionDirection, hitInfo.normal, material) * lightRadiance * NoL / pdf;
	}

	return radiance / samples;
}

Vector3 MonteCarloIntegrator::GenerateSampleVector(const Vector3& v, const Vector3& n, const BxDF& bxdf, const Material& material, float& pdf)
{
	// Generate random numbers which will decide our spherical coordinates
	float r1 = random.NextFloat();
	float r2 = random.NextFloat();

	// Calculate the sample vector in spherical coordinates
	float phi, theta;
	bxdf.GenerateSampleVector(Vector2(r1, r2), material, phi, theta, pdf);

	// Convert to carthesian coordinates and align with normal
	return TransformSampleVector(v, n, phi, theta);
}

Vector3 MonteCarloIntegrator::TransformSampleVector(const Vector3& v, const Vector3& n, float phi, float theta)
{
	assert(VectorUtil<3>::IsNormalized(v));
	assert(VectorUtil<3>::IsNormalized(n));

	// Convert to carthesian coordinates
	float sinTheta = sinf(theta);
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);

	Vector3 sample = Vector3(x, cosf(theta), z);

	assert(VectorUtil<3>::IsNormalized(sample));

	// Create an orientation matrix that aligns with the surface normal
	Vector3 right, forward;
	VectorUtil<3>::OrthoNormalize(n, v, right, forward);

	assert(VectorUtil<3>::IsNormalized(right));
	assert(VectorUtil<3>::IsNormalized(forward));

	assert(fabs(cml::dot(n, forward)) < 1e-5f);
	assert(fabs(cml::dot(n, right)) < 1e-5f);
	assert(fabs(cml::dot(right, forward)) < 1e-5f);

	Matrix33 transform(
		right[0], right[1], right[2],
		n[0], n[1], n[2],
		forward[0], forward[1], forward[2]
	);

	assert(cml::dot(sample, Vector3(0.0f, 1.0f, 0.0f)) >= 0.0f - 1e-5f);

	// Transform the sample to world space
	sample = transform_vector(transform, sample);

	assert(VectorUtil<3>::IsNormalized(sample));

	return sample;
}