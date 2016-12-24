#include "stdafx.h"

#include "debugintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"
#include "phongmaterial.h"
#include "sampler.h"

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
			color *= phongMaterial->diffuseMap->Sample(hitInfo.uv);

		return color.subvector(3);
	}

	//return Vector3(hitInfo.uv, 0);
	return hitInfo.normal;

	Vector3 lightDirection(1.0f, 1.0f, 0.0f);
	cml::normalize(lightDirection);

	return Vector3(1.0f, 1.0f, 1.0f) * Util::Clamp01(0.1f + VectorUtil<3>::Dot(hitInfo.normal, lightDirection));
}