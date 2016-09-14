#ifndef _WHITTED_INTEGRATOR_H_
#define _WHITTED_INTEGRATOR_H_

#include "surfaceintegrator.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class WhittedIntegrator : public SurfaceIntegrator
		{

		public:
			WhittedIntegrator(RayTracer& rayTracer);

			Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, int depth);
		};
	}
}

#endif