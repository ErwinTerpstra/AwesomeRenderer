#ifndef _SURFACE_INTEGRATOR_H_
#define _SURFACE_INTEGRATOR_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Material;

	class Renderable;

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
			
			virtual Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth) = 0;
			
		protected:
			Vector3 SampleDirectLight(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context);
			Vector3 SampleAreaLight(const Renderable* light);
			
		};
	}
}

#endif