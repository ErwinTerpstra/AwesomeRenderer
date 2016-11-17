#ifndef _MONTE_CARLO_INTEGRATOR_H_
#define _MONTE_CARLO_INTEGRATOR_H_

#include "surfaceintegrator.h"

namespace AwesomeRenderer
{
	class Random;

	namespace RayTracing
	{
		class BxDF;

		class MonteCarloIntegrator : public SurfaceIntegrator
		{
		public:
			uint32_t sampleCount;

		private:
			Random& random;

		public:
			MonteCarloIntegrator(RayTracer& rayTracer);

			Vector3 Li(const Ray& ray, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext, int depth);

		private:
			Vector3 Integrate(const Ray& ray, const RaycastHit& hitInfo, const Material& material, uint32_t samples, int depth);
			Vector3 Integrate(const Ray& ray, const RaycastHit& hitInfo, const BxDF& bxdf, const Material& material, uint32_t samples, int depth);

			Vector3 GenerateSampleVector(const Vector3& v, const Vector3& n, const BxDF& bxdf, const Material& material, float& pdf);
			Vector3 TransformSampleVector(const Vector3& v, const Vector3& n, float phi, float theta);

		};
	}
}

#endif