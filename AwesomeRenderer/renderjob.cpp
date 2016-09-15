#include "stdafx.h"

#include "raytracer.h"
#include "renderjob.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

RenderJob::RenderJob(RayTracer& rayTracer, std::vector<Point2>::iterator start, std::vector<Point2>::iterator end) : rayTracer(rayTracer), pixelIdx(0)
{
	pixelList.insert(pixelList.begin(), start, end);
}

void RenderJob::Run()
{
	while (pixelIdx < pixelList.size() && !IsInterrupted())
	{
		rayTracer.Render(pixelList[pixelIdx]);
		++pixelIdx;
	}
}

void RenderJob::Reset()
{
	WorkerJob::Reset();

	pixelIdx = 0;
}