#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

namespace AwesomeRenderer
{

	class RayTracer : public Renderer
	{

	public:

		RayTracer();

		void Render();

	private:
		
		void Trace(const Ray& ray, const Point2& screenPosition);
	};

}

#endif