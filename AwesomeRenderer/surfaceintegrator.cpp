#include "stdafx.h"
#include "surfaceintegrator.h"
#include "ray.h"
#include "raycasthit.h"
#include "lightdata.h"
#include "rendercontext.h"
#include "material.h"
#include "pbrmaterial.h"
#include "bsdf.h"
#include "bxdf.h"

#include "raytracer.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

SurfaceIntegrator::SurfaceIntegrator(RayTracer& rayTracer) : rayTracer(rayTracer)
{

}

Vector3 SurfaceIntegrator::SampleDirectLight(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context)
{
	const Vector3 wo = -ray.direction;
	const Vector3& normal = hitInfo.normal;
	
	Vector3 radiance(0.0f, 0.0f, 0.0f);
		
	// Iterate through all the lights
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = context.lightData->lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 wi;
		float distanceToLight;

		float intensity = light.intensity;

		if (light.type != LightData::DIRECTIONAL)
		{
			wi = light.position - hitInfo.point;

			distanceToLight = wi.length();
			wi.normalize();

			if (light.type == LightData::SPOT)
			{
				float angleTerm = VectorUtil<3>::Dot(light.direction, -wi);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity /= (light.constantAttenuation + light.lineairAttenuation * distanceToLight + light.quadricAttenuation * (distanceToLight * distanceToLight));
		}
		else
		{
			wi = -light.direction;
			distanceToLight = context.lightData->shadowDistance;
		}

		Ray shadowRay(hitInfo.point + hitInfo.normal * 1e-3f, wi);

		RaycastHit shadowHitInfo;
		if (rayTracer.RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;
		
		float NoL = std::max(VectorUtil<3>::Dot(normal, wi), 0.0f);
		Vector3 lightRadiance = light.color.subvector(3) * intensity;
		
		// TODO: This gets weird when the microfacet NDF returns a value > 1. Not sure how to handle this yet
		radiance += material.bsdf->Sample(wo, wi, normal, hitInfo, material, BSDF::BXDF_ALL) * lightRadiance * NoL;
	}
	
	return radiance;
}
