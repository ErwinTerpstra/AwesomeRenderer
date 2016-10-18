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

			Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth);

		private:
			Vector3 SampleReflection(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth);
			Vector3 SampleRefraction(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& context, int depth);

			float Fresnel(const Vector3& v, const Vector3& normal, float ior) const;
		};
	}
}

#endif