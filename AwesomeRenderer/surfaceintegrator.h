#ifndef _SURFACE_INTEGRATOR_H_
#define _SURFACE_INTEGRATOR_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Material;

	namespace RayTracing
	{
		class RayTracer;

		class SurfaceIntegrator
		{

		public:
			
		protected:
			RayTracer& rayTracer;

		public:
			SurfaceIntegrator(RayTracer& rayTracer);

			virtual Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, int depth) = 0;

		};
	}
}

#endif