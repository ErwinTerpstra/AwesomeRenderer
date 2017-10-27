#include "stdafx.h"
#include "montecarlointegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "microfacetmaterial.h"

#include "bxdf.h"
#include "bsdf.h"

#include "lambert.h"

#include "shadinginfo.h"
#include "lightdata.h"
#include "arealight.h"
#include "random.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

MonteCarloIntegrator::MonteCarloIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer), random(Random::instance)
{

}

Vector3 MonteCarloIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	const RenderContext& renderContext = rayTracer.GetRenderContext();
	Vector3 radiance = material.emission.subvector(3) * material.emissionIntensity;
	
	if (material.bsdf != NULL)
	{
		radiance += SampleDirectLight(ray, hitInfo, material, context);

		if (!renderContext.lightData->areaLights.empty())
		{
			// Select light
			for (int lightIdx = 0; lightIdx < renderContext.lightData->areaLights.size(); ++lightIdx)
			{
				const AreaLight* light = renderContext.lightData->areaLights[lightIdx];
				radiance += SampleAreaLight(*light, hitInfo.point, -ray.direction, hitInfo.normal, hitInfo, material);
			}
		}

		if (depth < rayTracer.maxDepth)
			radiance += Sample(hitInfo.point, -ray.direction, hitInfo.normal, hitInfo, material, depth);
	}

	if (material.translucent)
	{
		// Sample albedo because we need the surface alpha
		Color albedo = Lambert::SampleAlbedo(hitInfo, material, renderContext);

		// Create a 'refraction' ray, which actually just passes through the surface
		Ray refractionRay(hitInfo.point + ray.direction *1e-3f, ray.direction);
		ShadingInfo refractionShading;
		rayTracer.CalculateShading(refractionRay, refractionShading, depth);

		// Blend radiance for this surface with the refracted radiance
		Color color = Color(radiance, albedo[3]);
		ColorUtil::Blend(color, refractionShading.color, color);

		radiance = color.subvector(3);
	}
	
	return radiance;
}

Vector3 MonteCarloIntegrator::Sample(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth)
{
	// Sample BSDF
	Vector2 r(random.NextFloat(), random.NextFloat());

	Vector3 sampleVector;
	material.bsdf->GenerateSampleVector(r, wo, normal, material, sampleVector);
		
	float pdf = material.bsdf->CalculatePDF(wo, sampleVector, normal, material);

 	return Sample(p, wo, sampleVector, normal, hitInfo, material, depth, pdf);
}

Vector3 MonteCarloIntegrator::Sample(const Vector3& p, const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth, float pdf)
{
	if (pdf < 1e-5f)
		return Vector3(0.0f, 0.0f, 0.0f);

	float NoL = VectorUtil<3>::Dot(normal, wi);

	if (NoL <= 0.0f)
		return Vector3(0.0f, 0.0f, 0.0f);
	
	Vector3 reflectance = material.bsdf->Sample(wo, wi, normal, hitInfo, material, rayTracer.GetRenderContext());
	
	if (reflectance.length_squared() < 1e-5f)
		return Vector3(0.0f, 0.0f, 0.0f);

	// Calculate incoming light along this sample vector
	Ray reflectionRay(p + wi * 1e-5f, wi);

	ShadingInfo reflectionShading;
	rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

	Vector3 radiance = reflectionShading.color.subvector(3);

	return reflectance * radiance * NoL / pdf;
}

Vector3 MonteCarloIntegrator::SampleAreaLight(const AreaLight& light, const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material)
{
	Vector3 radiance(0.0f, 0.0f, 0.0f);

	Vector3 lightRadiance = light.material->emission.subvector(3) * light.material->emissionIntensity;

	// Sample light
	Vector3 lightNormal;
	Vector3 pointOnLight = light.primitive->Sample(p, Vector2(random.NextFloat(), random.NextFloat()), lightNormal);
	Vector3 toLight = pointOnLight - p;
	Vector3 lightSampleVector = VectorUtil<3>::Normalize(toLight);
	float distanceToLight = toLight.length();

	float lightPDF = light.primitive->CalculatePDF(p, lightSampleVector);
	float bsdfPDF = material.bsdf->CalculatePDF(wo, lightSampleVector, normal, material);

	// Shadow test
	RaycastHit lightHit;
	if (lightPDF > 0)
	{
		bool obstructed = rayTracer.RayCast(Ray(p + lightSampleVector * 1e-5f, lightSampleVector), lightHit, distanceToLight - 1e-5f);

		if (obstructed)
		{
			const AreaLight* hitElement = dynamic_cast<const AreaLight*>(lightHit.element);
			obstructed = hitElement != &light;
		}

		if (!obstructed)
		{
			Vector3 reflectance = material.bsdf->Sample(wo, lightSampleVector, normal, hitInfo, material, rayTracer.GetRenderContext());
			radiance += lightRadiance * reflectance * (VectorUtil<3>::Dot(normal, lightSampleVector) * PowerHeuristic(1, lightPDF, 1, bsdfPDF) / lightPDF);
		}
	}

	// Sample BSDF
	Vector3 bsdfSampleVector;
	material.bsdf->GenerateSampleVector(Vector2(random.NextFloat(), random.NextFloat()), wo, normal, material, bsdfSampleVector);
	lightPDF = light.primitive->CalculatePDF(p, bsdfSampleVector);
	bsdfPDF = material.bsdf->CalculatePDF(wo, bsdfSampleVector, normal, material);

	if (bsdfPDF > 0.0f)
	{
		Ray bsdfRay(p + bsdfSampleVector * 1e-5f, bsdfSampleVector);

		if (light.primitive->IntersectRay(bsdfRay, lightHit))
		{
			// Shadow test
			bool obstructed = rayTracer.RayCast(bsdfRay, lightHit, lightHit.distance);

			if (obstructed)
			{
				const AreaLight* hitElement = dynamic_cast<const AreaLight*>(lightHit.element);
				obstructed = hitElement != &light;
			}
			
			if (!obstructed)
			{
				Vector3 reflectance = material.bsdf->Sample(wo, bsdfSampleVector, normal, hitInfo, material, rayTracer.GetRenderContext());
				radiance += lightRadiance * reflectance * (VectorUtil<3>::Dot(normal, bsdfSampleVector) * PowerHeuristic(1, bsdfPDF, 1, lightPDF) / bsdfPDF);
			}
		}
	}

	return radiance;
}