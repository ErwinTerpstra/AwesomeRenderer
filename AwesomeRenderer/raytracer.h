#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "timer.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Window;

	class RayTracer : public Renderer
	{

	private:
		static const float MAX_FRAME_TIME;
		static const int MAX_DEPTH;

		struct ShadingInfo
		{
			Color color;
		};

		Timer timer;

		int pixelIdx;
		std::vector<Point2> pixelList;

	public:

		RayTracer();

		void Initialize();
		void Render();
		void Present(Window& window);
		void Cleanup();

		void SetRenderContext(const RenderContext* context);
	private:

		void PreRender();
		void PostRender();

		void Render(const Point2& pixel);
		void CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth = 0);

		bool RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance = FLT_MAX);
		void Trace(const Ray& ray, const Point2& screenPosition);
	};

}

#endif