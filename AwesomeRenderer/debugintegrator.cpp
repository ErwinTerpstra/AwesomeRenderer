#include "stdafx.h"

#include "debugintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "rendercontext.h"
#include "texture.h"

#include "material.h"
#include "microfacetmaterial.h"
#include "phongmaterial.h"
#include "sampler.h"
#include "shadinginfo.h"

#include "inputmanager.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

DebugIntegrator::DebugIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer)
{

}


Vector3 DebugIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	if (phongMaterial != NULL)
	{
		Color color = phongMaterial->diffuseColor;

		if (phongMaterial->diffuseMap != NULL)
		{
			if (InputManager::Instance().GetKey('M'))
				color *= phongMaterial->diffuseMap->Sample(hitInfo.uv, 0U);
			else
				color *= phongMaterial->diffuseMap->SampleMipMaps(hitInfo.uv, hitInfo.distance, hitInfo.surfaceAreaToTextureRatio, context.renderTarget->frameBuffer->GetResolution());
		}

		if (material.translucent)
		{
			Ray refractionRay(hitInfo.point + ray.direction *1e-3f, ray.direction);
			ShadingInfo refractionShading;
			rayTracer.CalculateShading(refractionRay, refractionShading, depth);

			ColorUtil::Blend(color, refractionShading.color, color);
		}

		return color.subvector(3);
	}

	// Depth
	float depthColor = hitInfo.distance / 1000.0f;
	return Vector3(depthColor, depthColor, depthColor);

	// Normals
	return Vector3(0.5f, 0.5f, 0.5f) + hitInfo.normal * 0.5f;

	// Basic diffuse light
	Vector3 lightDirection(1.0f, 1.0f, 0.0f);
	cml::normalize(lightDirection);

	return Vector3(1.0f, 1.0f, 1.0f) * Util::Clamp01(0.1f + VectorUtil<3>::Dot(hitInfo.normal, lightDirection));
}