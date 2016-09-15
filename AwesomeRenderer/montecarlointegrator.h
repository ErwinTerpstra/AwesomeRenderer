#ifndef _MONTE_CARLO_INTEGRATOR_H_
#define _MONTE_CARLO_INTEGRATOR_H_

#include "surfaceintegrator.h"

namespace AwesomeRenderer
{
	class Random;

	namespace RayTracing
	{

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
			Vector3 GenerateSampleVector(const Vector3& v, const Vector3& n, float roughness, float& pdf);
			void ImportanceSampleGGX(const Vector2& r, float alpha, float& phi, float& theta);
			float PDFGGX(float phi, float theta, float alpha);
		};
	}
}

#endif