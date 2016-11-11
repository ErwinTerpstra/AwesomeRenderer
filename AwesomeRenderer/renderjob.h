#ifndef _RENDER_JOB_H_
#define _RENDER_JOB_H_

#include "awesomerenderer.h"

#include "threading.h"
#include "workerjob.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class RayTracer;

		class RenderJob : public WorkerJob
		{

		private:
			RayTracer& rayTracer;

			uint32_t pixelIdx;
			uint32_t x, y, width, height;

		public:
			RenderJob(RayTracer& rayTracer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
			RenderJob(RayTracer& rayTracer, std::vector<Point2>::iterator start, std::vector<Point2>::iterator end);
			
			void Reset();

			float GetProgress() const { return pixelIdx / (float)(width * height); }

		protected:
			void Run();

		};

	}
}

#endif