#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"

namespace AwesomeRenderer
{
	class Ray;
	class Window;

	class RayTracer : public Renderer
	{

	public:

		RayTracer();

		void Initialize();
		void Render();
		void Present(Window& window);
		void Cleanup();
	private:

		void PreRender();
		void PostRender();

		void Trace(const Ray& ray, const Point2& screenPosition);
	};

}

#endif