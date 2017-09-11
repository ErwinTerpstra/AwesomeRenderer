#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "timer.h"

#include "debugintegrator.h"
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
	class MicrofacetMaterial;

	class Scheduler;
	class JobGroup;

	class Random;

	namespace RayTracing
	{
		class RenderJob;

		class RayTracer : public Renderer
		{
			friend class RenderJob;

		private:
			static const uint32_t MAX_FRAME_TIME;
			static const uint32_t TILE_SIZE;
			static const uint32_t SUBPIXEL_STRATIFICATION_SIZE;

			Timer frameTimer;

			JobGroup* jobGroup;

			std::vector<Point2> pixelList;
			std::vector<RenderJob*> renderJobs;

			bool renderingFrame;

		public:

			DebugIntegrator debugIntegrator;
			WhittedIntegrator whittedIntegrator;
			MonteCarloIntegrator monteCarloIntegrator;
			SurfaceIntegrator* currentIntegrator;

			uint32_t maxDepth;
			uint32_t samplesPerPixel;
			uint32_t renderedSamples;

			Point2 debugPixel;

			Random& random;

		public:

			RayTracer(Scheduler& scheduler);

			void Initialize();
			void Render();
			void Present(Window& window);
			void Cleanup();
			void ResetFrame(bool startNewFrame = true);

			void SetRenderContext(const RenderContext* context);

			void BreakOnDebugPixel(const Point2& pixel);

			void Render(const Point2& pixel);
			bool CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth = 0) const;
			bool RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance = FLT_MAX) const;

			float GetProgress() const;
			bool IsRenderingFrame() const { return renderingFrame; }
			float FrameTime() const { return frameTimer.Poll(); }
		private:

			void PreRender();
			void PostRender();

		};

	}

}

#endif