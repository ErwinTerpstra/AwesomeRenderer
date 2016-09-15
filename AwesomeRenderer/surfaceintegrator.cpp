#include "stdafx.h"
#include "surfaceintegrator.h"
#include "ray.h"
#include "raycasthit.h"
#include "lightdata.h"
#include "rendercontext.h"
#include "material.h"

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
	// TODO: make this functionality of the base class SurfaceIntegrator. Then each integrator can decide whether it wants direct lighting or not
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
				float angleTerm = cml::dot(light.direction, -toLight);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity /= (distanceToLight * distanceToLight);
		}
		else
		{
			toLight = -light.direction;
			distanceToLight = 1000.0f; // TODO: shadow distance render context parameter?
		}

		Ray shadowRay(hitInfo.point + toLight * 1e-5f, toLight);

		RaycastHit shadowHitInfo;
		if (rayTracer.RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;

		float NoL = std::max(cml::dot(normal, toLight), 0.0f);
		Vector3 lightRadiance = light.color.subvector(3) * intensity;

		radiance += material.bsdf->Sample(viewVector, toLight, normal, material) * lightRadiance * NoL;
	}

	/*/
	if (FALSE)
	{
		float ior = 0.75f;

		// Refraction
		Vector3 innerRefractionDirection;
		VectorUtil<3>::Refract(ray.direction, normal, ior, innerRefractionDirection);

		Ray innerRefractionRay(hitInfo.point - normal * 0.01f, innerRefractionDirection);

		Ray refractionRay;

		RaycastHit refractionHit;
		if (RayCast(innerRefractionRay, refractionHit))
		{
		if (refractionHit.inside)
		{
		ior = 1.0f / ior;

		Vector3 outerRefractionDirection;
		VectorUtil<3>::Refract(innerRefractionDirection, refractionHit.normal, ior, outerRefractionDirection);

		refractionRay = Ray(refractionHit.point + normal * 0.01f, outerRefractionDirection);
		}
		else
		{
		refractionRay = innerRefractionRay;
		}

		}

		ShadingInfo refractionShading;
		CalculateShading(refractionRay, refractionShading, depth + 1);

		radiance += refractionShading.color.subvector(3);
	}
	/**/

	return radiance;
}