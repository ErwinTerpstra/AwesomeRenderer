#ifndef _DEBUG_INTEGRATOR_H_
#define _DEBUG_INTEGRATOR_H_

#include "awesomerenderer.h"
#include "surfaceintegrator.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{

		class DebugIntegrator : public SurfaceIntegrator
		{

		public:
			DebugIntegrator(RayTracer& rayTracer);

			Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth);

		};
	}
}

#endif