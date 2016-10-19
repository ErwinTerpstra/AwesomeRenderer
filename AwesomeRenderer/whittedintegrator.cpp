#include "stdafx.h"
#include "whittedintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"

#include "shadinginfo.h"

#include "sphere.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

WhittedIntegrator::WhittedIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer)
{

}

Vector3 WhittedIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 radiance = material.emission.subvector(3);
	
	radiance += SampleDirectLight(ray, hitInfo, material, context);

	if (depth < rayTracer.maxDepth)
	{
		Vector3 reflection = SampleReflection(ray, hitInfo, material, context, depth);

		if (material.translucent)
		{
			Vector3 refraction = SampleRefraction(ray, hitInfo, material, context, depth);

			float fresnel = Fresnel(ray.direction, hitInfo.normal, material.ior);
			radiance += reflection * fresnel + refraction * (1.0f - fresnel);
		}
		else
			radiance += reflection;
	}

	return radiance;
}

Vector3 WhittedIntegrator::SampleReflection(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 reflectionDirection;
	VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

	// Reflection
	Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-3f, reflectionDirection);

	ShadingInfo reflectionShading;
	rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

	assert(fabs(cml::dot(hitInfo.normal, reflectionDirection) - cml::dot(hitInfo.normal, -ray.direction)) < 1e-5f);

	float NoL = Util::Clamp01(cml::dot(hitInfo.normal, reflectionDirection));
	Vector3 lightRadiance = reflectionShading.color.subvector(3);

	return material.bsdf->Sample(-ray.direction, reflectionDirection, hitInfo.normal, material) * lightRadiance * NoL;
}

Vector3 WhittedIntegrator::SampleRefraction(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	if (cml::dot(-ray.direction, hitInfo.normal) < 1e-3f)
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
		if (refractionHit.inside)
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

float WhittedIntegrator::Fresnel(const Vector3& v, const Vector3& normal, float ior) const
{
	float cosi = cml::dot(v, normal);
	float etai = 1, etat = ior;

	if (cosi > 0)
		std::swap(etai, etat);

	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));

	// Total internal reflection
	if (sint >= 1)
		return 1.0f;

	float cost = sqrtf(std::max(0.f, 1 - sint * sint));
	cosi = fabsf(cosi);

	float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
	float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

	return (Rs * Rs + Rp * Rp) / 2;
}