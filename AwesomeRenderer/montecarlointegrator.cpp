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
#include "lightdata.h"
#include "renderable.h"
#include "random.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MonteCarloIntegrator::MonteCarloIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer), sampleCount(1), random(Random::instance)
{

}

Vector3 MonteCarloIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 radiance = material.emission.subvector(3) * PI;
	
	if (material.bsdf != NULL)
	{
		radiance += SampleDirectLight(ray, hitInfo, material, context);

		if (depth < rayTracer.maxDepth)
			radiance += Sample(hitInfo.point, -ray.direction, hitInfo.normal, hitInfo, material, depth);
	}
	
	return radiance;
}

Vector3 MonteCarloIntegrator::Sample(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth)
{
	const RenderContext& renderContext = rayTracer.GetRenderContext();
	if (!renderContext.lightData->areaLights.empty())
	{
		// Sample light
		int lightIdx = random.NextInt(0, renderContext.lightData->areaLights.size());
		assert(lightIdx >= 0 && lightIdx < renderContext.lightData->areaLights.size());

		const Renderable* light = renderContext.lightData->areaLights[lightIdx];
		const Primitive& primitive = light->GetPrimitive();
		
		Vector3 lightNormal;
		Vector3 pointOnLight = primitive.Sample(p, Vector2(random.NextFloat(), random.NextFloat()), lightNormal);
		
		Vector3 lightSampleVector = VectorUtil<3>::Normalize(pointOnLight - p);
		float lightPDF = primitive.CalculatePDF(p, lightSampleVector);

		Vector3 bsdfSampleVector;
		material.bsdf->GenerateSampleVector(Vector2(random.NextFloat(), random.NextFloat()), wo, normal, material, bsdfSampleVector);
		float bsdfPDF = material.bsdf->CalculatePDF(wo, bsdfSampleVector, normal, material);

		Vector3 radiance(0.0f, 0.0f, 0.0f);
				
		/*
		if (random.NextFloat() < 0.5f)
			radiance += Sample(p, wo, lightSampleVector, normal, hitInfo, material, rayTracer.maxDepth, lightPDF);
		else
			radiance += Sample(p, wo, bsdfSampleVector, normal, hitInfo, material, depth, bsdfPDF);
		*/

		// TODO: Sample light directly, instead of traversing the whole scene. Perform only shadow test
		radiance += Sample(p, wo, lightSampleVector, normal, hitInfo, material, rayTracer.maxDepth, lightPDF) * PowerHeuristic(1, lightPDF, 1, bsdfPDF);
		radiance += Sample(p, wo, bsdfSampleVector, normal, hitInfo, material, depth, bsdfPDF) * PowerHeuristic(1, bsdfPDF, 1, lightPDF);

		return radiance;
	}
	else
	{
		// Sample BSDF
		Vector2 r(random.NextFloat(), random.NextFloat());

		Vector3 sampleVector;
		material.bsdf->GenerateSampleVector(r, wo, normal, material, sampleVector);
		
		float pdf = material.bsdf->CalculatePDF(wo, sampleVector, normal, material);

		return Sample(p, wo, sampleVector, normal, hitInfo, material, depth, pdf);
	}
}

Vector3 MonteCarloIntegrator::Sample(const Vector3& p, const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth, float pdf)
{
	if (pdf < 1e-5f)
		return Vector3(0.0f, 0.0f, 0.0f);

	float NoL = VectorUtil<3>::Dot(normal, wi);

	if (NoL <= 0.0f)
		return Vector3(0.0f, 0.0f, 0.0f);
	
	Vector3 h = cml::normalize(wo + wi);
	
	Vector3 reflectance = material.bsdf->Sample(wo, wi, normal, hitInfo, material);
	
	if (reflectance.length_squared() < 1e-5f)
		return Vector3(0.0f, 0.0f, 0.0f);

	// Calculate incoming light along this sample vector
	Ray reflectionRay(p + normal * 1e-3f, wi);

	ShadingInfo reflectionShading;
	float distance;

	if (rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1))
		distance = (p - reflectionShading.hitInfo.point).length();
	else
		distance = 0.0f;

	// Attenuate by distance
	Vector3 radiance = reflectionShading.color.subvector(3);// / (1.0f + distance * distance);

	return reflectance * radiance * NoL / pdf;
}

Vector3 MonteCarloIntegrator::Integrate(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, uint32_t samples, int depth)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);
		
	for (uint32_t sampleIdx = 0; sampleIdx < samples; ++sampleIdx)
	{
		radiance += Sample(p, wo, normal, hitInfo, material, depth);
	}

	return radiance / samples;
}
