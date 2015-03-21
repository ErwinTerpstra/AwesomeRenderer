#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "timer.h"

namespace AwesomeRenderer
{
	class Ray;
	class Window;

	class RayTracer : public Renderer
	{

	private:
		static const float MAX_FRAME_TIME;

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

		void Trace(const Ray& ray, const Point2& screenPosition);
	};

}

#endif