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
		radiance += Integrate(hitInfo.point, -ray.direction, hitInfo.normal, hitInfo, material, sampleCount, depth);
	
	return radiance;
}

Vector3 MonteCarloIntegrator::Integrate(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, uint32_t samples, int depth)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);

	// Determine the BxDF that will control sample vectors
	const BxDF* sampleBxDF;
	if (material.bsdf->specular != NULL)
		sampleBxDF = material.bsdf->specular;
	else if (material.bsdf->diffuse != NULL)
		sampleBxDF = material.bsdf->diffuse;
	else
		return radiance;
	
	for (uint32_t sampleIdx = 0; sampleIdx < samples; ++sampleIdx)
	{
		Vector2 r(random.NextFloat(), random.NextFloat());

		Vector3 sampleVector;
		sampleBxDF->GenerateSampleVector(r, wo, normal, material, sampleVector);
		
		float NoL = VectorUtil<3>::Dot(normal, sampleVector);

		if (NoL <= 0.0f)
			continue;

		Vector3 h = cml::normalize(wo + sampleVector);

		// Sample specular BRDF
		Vector3 specularReflectance(0.0f, 0.0f, 0.0f);
		if (material.bsdf->specular != NULL)
		{
			float pdf = material.bsdf->specular->CalculatePDF(wo, sampleVector, normal, material);

			if (pdf > 0.0f)
				specularReflectance = material.bsdf->specular->Sample(wo, sampleVector, normal, hitInfo, material) / pdf;
		}

		// Sample diffuse BRDF
		Vector3 diffuseReflectance(0.0f, 0.0f, 0.0f);
		if (material.bsdf->diffuse != NULL)
		{
			float pdf = material.bsdf->diffuse->CalculatePDF(wo, sampleVector, normal, material);

			if (pdf > 0.0f)
				diffuseReflectance = material.bsdf->diffuse->Sample(wo, sampleVector, normal, hitInfo, material) / pdf;
		}

		// Combine in a single reflectance value
		Vector3 reflectance = material.bsdf->SpecularTradeoff(diffuseReflectance, specularReflectance, normal, h, material);

		if (reflectance.length_squared() < 1e-5f)
			continue;

		// Calculate incoming light along this sample vector
		Ray reflectionRay(p + normal * 1e-3f, sampleVector);
		
		ShadingInfo reflectionShading;
		rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);
		
		Vector3 lightRadiance = reflectionShading.color.subvector(3);

		radiance += reflectance * lightRadiance * NoL;
	}

	return radiance / samples;
}

Vector3 MonteCarloIntegrator::Integrate(const Ray& ray, const RaycastHit& hitInfo, const BxDF& bxdf, const Material& material, uint32_t samples, int depth)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);
	
	for (uint32_t sampleIdx = 0; sampleIdx < samples; ++sampleIdx)
	{
		Vector2 r(random.NextFloat(), random.NextFloat());
		
		Vector3 reflectionDirection;
		bxdf.GenerateSampleVector(r, -ray.direction, hitInfo.normal, material, reflectionDirection);

		float pdf = bxdf.CalculatePDF(-ray.direction, reflectionDirection, hitInfo.normal, material);
		
		if (pdf < 1e-5f)
			continue;
		
		Vector3 reflectance = bxdf.Sample(-ray.direction, reflectionDirection, hitInfo.normal, hitInfo, material);
		
		// Calculate incoming light along this sample vector
		Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-3f, reflectionDirection);

		ShadingInfo reflectionShading;
		rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

		float NoL = VectorUtil<3>::Dot(hitInfo.normal, reflectionDirection);
		assert(NoL + 1e-5f >= 0.0f && NoL - 1e-5f <= 1.0f);

		Vector3 lightRadiance = reflectionShading.color.subvector(3);

		radiance += reflectance * lightRadiance * NoL * (1.0f / pdf);
	}

	return radiance / samples;
}
