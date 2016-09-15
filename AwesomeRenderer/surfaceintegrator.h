#ifndef _SURFACE_INTEGRATOR_H_
#define _SURFACE_INTEGRATOR_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Material;

	class RenderContext;

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

			Vector3 SampleDirectLight(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context);

			virtual Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth) = 0;

		};
	}
}

#endif