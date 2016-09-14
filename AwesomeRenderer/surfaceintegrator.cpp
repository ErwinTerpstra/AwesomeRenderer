#include "stdafx.h"
#include "surfaceintegrator.h"

#include "raytracer.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

SurfaceIntegrator::SurfaceIntegrator(RayTracer& rayTracer) : rayTracer(rayTracer)
{

}