#include "stdafx.h"
#include "whittedintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "microfacetmaterial.h"

#include "shadinginfo.h"

#include "sphere.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

WhittedIntegrator::WhittedIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer)
{

}

Vector3 WhittedIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 radiance = material.emission.subvector(3) * material.emissionIntensity;
	
	if (material.bsdf != NULL)
	{
		radiance += SampleDirectLight(ray, hitInfo, material, context);

		if (depth < rayTracer.maxDepth)
		{
			Vector3 reflection = SampleReflection(ray, hitInfo, material, context, depth);

			if (material.translucent)
			{
				Vector3 refraction = SampleRefraction(ray, hitInfo, material, context, depth);

				float fresnel = RenderUtil::Fresnel(ray.direction, hitInfo.normal, material.ior);
				radiance += reflection * fresnel + refraction * (1.0f - fresnel);
			}
			else
				radiance += reflection;
		}
	}

	return radiance;
}

Vector3 WhittedIntegrator::SampleReflection(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 reflectionDirection;
	VectorUtil<3>::Reflect(-ray.direction, hitInfo.normal, reflectionDirection);

	// Reflection
	Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-3f, reflectionDirection);

	ShadingInfo reflectionShading;
	rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

	assert(fabs(VectorUtil<3>::Dot(hitInfo.normal, reflectionDirection) - VectorUtil<3>::Dot(hitInfo.normal, -ray.direction)) < 1e-5f);

	float NoL = Util::Clamp01(VectorUtil<3>::Dot(hitInfo.normal, reflectionDirection));
	Vector3 lightRadiance = reflectionShading.color.subvector(3);

	float pdf = material.bsdf->CalculatePDF(-ray.direction, reflectionDirection, hitInfo.normal, material);

	return material.bsdf->Sample(-ray.direction, reflectionDirection, hitInfo.normal, hitInfo, material, rayTracer.GetRenderContext()) * lightRadiance * NoL / pdf;
}

Vector3 WhittedIntegrator::SampleRefraction(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	if (VectorUtil<3>::Dot(-ray.direction, hitInfo.normal) < 1e-3f)
		return Vector3(0.0f, 0.0f, 0.0f);

	float ior = material.ior;

	// Refraction
	Vector3 innerRefractionDirection;
	VectorUtil<3>::Refract(ray.direction, hitInfo.normal, ior, innerRefractionDirection);

	Ray innerRefractionRay(hitInfo.point + innerRefractionDirection * 1e-3f, innerRefractionDirection);
	Ray refractionRay;

	RaycastHit refractionHit;
	if (rayTracer.RayCast(innerRefractionRay, refractionHit))
	{
		bool inside = VectorUtil<3>::Dot(refractionHit.normal, innerRefractionRay.direction) > 0;

		if (inside)
		{
			ior = 1.0f / ior;

			Vector3 outerRefractionDirection;
			VectorUtil<3>::Refract(innerRefractionDirection, -refractionHit.normal, ior, outerRefractionDirection);

			refractionRay = Ray(refractionHit.point + outerRefractionDirection * 1e-3f, outerRefractionDirection);
		}
		else
		{
			refractionRay = innerRefractionRay;
		}

	}

	ShadingInfo refractionShading;
	rayTracer.CalculateShading(refractionRay, refractionShading, depth + 1);

	return refractionShading.color.subvector(3);
}
