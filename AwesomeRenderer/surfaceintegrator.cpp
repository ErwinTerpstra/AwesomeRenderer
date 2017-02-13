#include "stdafx.h"
#include "surfaceintegrator.h"
#include "ray.h"
#include "raycasthit.h"
#include "lightdata.h"
#include "rendercontext.h"
#include "material.h"
#include "pbrmaterial.h"

#include "raytracer.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

SurfaceIntegrator::SurfaceIntegrator(RayTracer& rayTracer) : rayTracer(rayTracer)
{

}

Vector3 SurfaceIntegrator::SampleDirectLight(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context)
{
	const Vector3 viewVector = -ray.direction;
	const Vector3& normal = hitInfo.normal;
	
	Vector3 radiance(0.0f, 0.0f, 0.0f);
		
	// Iterate through all the lights
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = context.lightData->lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 toLight;
		float distanceToLight;

		float intensity = light.intensity;

		if (light.type != LightData::DIRECTIONAL)
		{
			toLight = light.position - hitInfo.point;

			distanceToLight = toLight.length();
			toLight.normalize();

			if (light.type == LightData::SPOT)
			{
				float angleTerm = VectorUtil<3>::Dot(light.direction, -toLight);
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
			toLight = -light.direction;
			distanceToLight = 10000.0f; // TODO: shadow distance render context parameter?
		}

		Ray shadowRay(hitInfo.point + hitInfo.normal * 1e-3f, toLight);

		RaycastHit shadowHitInfo;
		if (rayTracer.RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;
		
		float NoL = std::max(VectorUtil<3>::Dot(normal, toLight), 0.0f);
		Vector3 lightRadiance = light.color.subvector(3) * intensity;
		
		radiance += material.bsdf->Sample(viewVector, toLight, normal, hitInfo, material) * lightRadiance * NoL;
	}
	
	return radiance;
}