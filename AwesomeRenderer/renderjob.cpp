#include "stdafx.h"

#include "raytracer.h"
#include "renderjob.h"

#include "camera.h"
#include "rendercontext.h"
#include "shadinginfo.h"
#include "texture.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;


RenderJob::RenderJob(RayTracer& rayTracer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) : 
	rayTracer(rayTracer), x(x), y(y), width(width), height(height), pixelIdx(0)
{
}

void RenderJob::Run()
{
	const RenderContext& renderContext = rayTracer.GetRenderContext();
	Buffer* frameBuffer = renderContext.renderTarget->frameBuffer;

	uint32_t pixels = width * height;

	while (pixelIdx < pixels && !IsInterrupted())
	{
		Point2 pixel(x + (pixelIdx % width), y + (pixelIdx / width));

		rayTracer.Render(pixel);

		/*
		rayTracer.BreakOnDebugPixel(pixel);

		// Create a ray from the camera near plane through this pixel
		Ray primaryRay;
		renderContext.camera->ViewportToRay(pixel, primaryRay);

		ShadingInfo shadingInfo;
		rayTracer.CalculateShading(primaryRay, shadingInfo);

		// Write to color buffer
		frameBuffer->SetPixel(pixel[0], pixel[1], shadingInfo.color);
		*/

		++pixelIdx;
	}
}

void RenderJob::Reset()
{
	WorkerJob::Reset();

	pixelIdx = 0;
}