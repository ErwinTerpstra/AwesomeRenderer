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
			Vector3 Sample(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth);
			Vector3 Sample(const Vector3& p, const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, int depth, float pdf);

			Vector3 Integrate(const Vector3& p, const Vector3& wo, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, uint32_t samples, int depth);
			
			AR_INLINE float BalanceHeuristic(int nF, float pdfF, int nG, float pdfG)
			{
				return (nF * pdfF) / (nF * pdfF + nG * pdfG);
			}

			AR_INLINE float PowerHeuristic(int nF, float pdfF, int nG, float pdfG)
			{
				float f = nF * pdfF, g = nG * pdfG;

				return (f * f) / (f * f + g * g);
			}
		};
	}
}

#endif