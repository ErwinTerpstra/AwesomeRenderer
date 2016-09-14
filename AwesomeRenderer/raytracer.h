#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "timer.h"

#include "whittedintegrator.h"
#include "montecarlointegrator.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	struct ShadingInfo;

	class Window;
	class Random;

	class PhongMaterial;
	class PbrMaterial;

	namespace RayTracing
	{
		class RayTracer : public Renderer
		{
		private:
			static const float MAX_FRAME_TIME;

			Timer timer;
			Timer frameTimer;

			uint32_t pixelIdx;
			std::vector<Point2> pixelList;

		public:

			WhittedIntegrator whittedIntegrator;
			MonteCarloIntegrator monteCarloIntegrator;

			uint32_t maxDepth;

		public:

			RayTracer();

			void Initialize();
			void Render();
			void Present(Window& window);
			void Cleanup();
			void ResetFrame();

			void SetRenderContext(const RenderContext* context);

			void CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth = 0);

			float GetProgress() const { return pixelIdx / (float)pixelList.size(); }
		private:

			void PreRender();
			void PostRender();

			void Render(const Point2& pixel);

			void CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const Material& material, ShadingInfo& shadingInfo, int depth);

			bool RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance = FLT_MAX);
			void Trace(const Ray& ray, const Point2& screenPosition);

			float Fresnel(const Vector3& v, const Vector3& normal, float ior);

		};

	}

}

#endif