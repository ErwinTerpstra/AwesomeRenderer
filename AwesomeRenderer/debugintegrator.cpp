#include "stdafx.h"

#include "debugintegrator.h"

#include "raytracer.h"

#include "ray.h"
#include "raycasthit.h"

#include "material.h"
#include "pbrmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

DebugIntegrator::DebugIntegrator(RayTracer& rayTracer) : SurfaceIntegrator(rayTracer)
{

}


Vector3 DebugIntegrator::Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth)
{
	return hitInfo.normal;

	Vector3 lightDirection(1.0f, 1.0f, 0.0f);
	cml::normalize(lightDirection);

	return Vector3(1.0f, 1.0f, 1.0f) * Util::Clamp01(0.1f + cml::dot(hitInfo.normal, lightDirection));
}