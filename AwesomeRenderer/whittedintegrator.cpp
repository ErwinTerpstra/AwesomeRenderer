#include "stdafx.h"
#include "whittedintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"

#include "shadinginfo.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

WhittedIntegrator::WhittedIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer)
{

}

Vector3 WhittedIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	Vector3 radiance = SampleDirectLight(ray, hitInfo, material, context);

	if (depth < rayTracer.maxDepth)
	{
		Vector3 reflectionDirection;
		VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

		// Reflection
		Ray reflectionRay(hitInfo.point + hitInfo.normal * 1e-5f, reflectionDirection);

		ShadingInfo reflectionShading;
		rayTracer.CalculateShading(reflectionRay, reflectionShading, depth + 1);

		assert(fabs(cml::dot(hitInfo.normal, reflectionDirection) - cml::dot(hitInfo.normal, -ray.direction)) < 1e-5f);

		float NoL = Util::Clamp01(cml::dot(hitInfo.normal, reflectionDirection));
		Vector3 lightRadiance = reflectionShading.color.subvector(3);

		radiance += material.bsdf->Sample(-ray.direction, reflectionDirection, hitInfo.normal, material) * lightRadiance * NoL;
	}

	return radiance;
}