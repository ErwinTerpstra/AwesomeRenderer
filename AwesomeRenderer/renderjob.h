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
			std::vector<Point2> pixelList;

		public:
			RenderJob(RayTracer& rayTracer, std::vector<Point2>::iterator start, std::vector<Point2>::iterator end);
			
			void Reset();

			float GetProgress() const { return pixelIdx / (float)pixelList.size(); }

		protected:
			void Run();

		};

	}
}

#endif