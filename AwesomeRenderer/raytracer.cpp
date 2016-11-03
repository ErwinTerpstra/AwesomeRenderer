#include "stdafx.h"
#include "node.h"
#include "raytracer.h"
#include "ray.h"
#include "raycasthit.h"
#include "buffer.h"
#include "camera.h"
#include "gdibuffer.h"
#include "window.h"
#include "primitive.h"
#include "model.h"
#include "renderable.h"
#include "material.h"
#include "phongmaterial.h"
#include "pbrmaterial.h"
#include "lightdata.h"
#include "skybox.h"
#include "random.h"
#include "shadinginfo.h"
#include "renderjob.h"
#include "scheduler.h"
#include "jobgroup.h"

#include "inputmanager.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

const float RayTracer::MAX_FRAME_TIME = 0.05f;
const uint32_t RayTracer::PIXELS_PER_JOB = 4096;

RayTracer::RayTracer(Scheduler& scheduler) : Renderer(), debugIntegrator(*this), whittedIntegrator(*this), monteCarloIntegrator(*this), renderingFrame(false), maxDepth(0), downScale(0), frameTimer(0.0f, FLT_MAX)
{
	currentIntegrator = &debugIntegrator;
	
	jobGroup = scheduler.CreateJobGroup(8);
}

void RayTracer::Initialize()
{
	
}

void RayTracer::SetRenderContext(const RenderContext* context)
{
	if (context == renderContext)
		return;

	assert(renderContext == NULL && "RayTracer can't switch render contexts!"); // TODO: Handle this more gracefully

	Renderer::SetRenderContext(context);

	Buffer* frameBuffer = context->renderTarget->frameBuffer;
	
	// Add all the pixel coordinates to the list
	for (uint32_t y = 0; y < frameBuffer->height; y += 1 << downScale)
	{
		for (uint32_t x = 0; x < frameBuffer->width; x += 1 << downScale)
			pixelList.push_back(Point2(x, y));
	}

	uint32_t pixels = pixelList.size();

	std::random_shuffle(pixelList.begin(), pixelList.end());

	// Create render jobs to hold all pixels
	uint32_t renderJobCount = ceil(pixels / (float)PIXELS_PER_JOB);

	std::vector<Point2>::iterator it = pixelList.begin();
	for (uint32_t renderJobIdx = 0; renderJobIdx < renderJobCount; ++renderJobIdx)
	{
		RenderJob* job = new RenderJob(*this, it + renderJobIdx * PIXELS_PER_JOB, it + std::min((renderJobIdx + 1) * PIXELS_PER_JOB, pixels));
		renderJobs.push_back(job);
	}

	//std::random_shuffle(renderJobs.begin(), renderJobs.end());
}

void RayTracer::PreRender()
{
	frameTimer.Tick();

	// Schedule all render jobs
	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
		jobGroup->EnqueueJob(*it);

	renderingFrame = true;
}

void RayTracer::PostRender()
{
	float time = frameTimer.Poll();
	
	// Prevent new jobs from starting
	jobGroup->ClearQueue();

	// Resetting also waits for interrupts, but it's better to interrupt all jobs at once. This way running jobs can finish at the same time instead of one by one.
	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
		(*it)->Interrupt();

	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
		(*it)->Reset();

	renderingFrame = false;

	printf("[RayTracer]: Rendered frame in %.0fms.\n", time * 1000);
}

void RayTracer::Render()
{
	if (!renderingFrame)
		PreRender();

	Sleep(MAX_FRAME_TIME);

	bool allCompleted = true;
	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
	{
		if (!(*it)->IsCompleted())
		{
			allCompleted = false;
			break;
		}
	}

	if (allCompleted)
		PostRender();
}

void RayTracer::Present(Window& window)
{
	GdiBuffer* buffer = static_cast<GdiBuffer*>(renderContext->renderTarget->frameBuffer);

	if (buffer != NULL)
		window.DrawBuffer(*buffer);
}

void RayTracer::Cleanup()
{
	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
	{
		RenderJob* job = *it;
		job->Interrupt();
		job->WaitForCompletion();

		delete job;
	}

	renderJobs.clear();
}

void RayTracer::ResetFrame()
{
	PostRender();

	renderContext->renderTarget->Clear(Color::BLACK, renderContext->clearFlags);

	PreRender();
}

float RayTracer::GetProgress() const
{
	float progress = 0.0f;

	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
		progress += (*it)->GetProgress();

	return progress / renderJobs.size();
}

void RayTracer::Render(const Point2& pixel)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;

	// Create a ray from the camera near plane through this pixel
	Ray primaryRay;
	renderContext->camera->ViewportToRay(pixel, primaryRay);

	ShadingInfo shadingInfo;
	CalculateShading(primaryRay, shadingInfo);

	if (downScale > 0)
	{
		// Write to color buffer
		for (int y = 0; y < (1 << downScale); ++y)
			for (int x = 0; x < (1 << downScale); ++x)
				frameBuffer->SetPixel(std::min(pixel[0] + x, (int) frameBuffer->width - 1), std::min(pixel[1] + y, (int) frameBuffer->height - 1), shadingInfo.color);
	}
	else
	{
		// Write to color buffer
		frameBuffer->SetPixel(pixel[0], pixel[1], shadingInfo.color);
	}
}

void RayTracer::CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth) const
{
	const LightData& lightData = *renderContext->lightData;

	// Perform the raycast to find out which node we've hit
	RaycastHit hitInfo;
	if (!RayCast(ray, hitInfo))
	{
		if (renderContext->skybox != NULL)
			renderContext->skybox->Sample(ray.direction, shadingInfo.color);

		return;
	}

	const Renderable* renderable = dynamic_cast<const Renderable*>(hitInfo.element);
	const Material* material = renderable->material;

	assert(material->bsdf != NULL);
	
	shadingInfo.color = Color(currentIntegrator->Li(ray, hitInfo, *material, *renderContext, depth), 1.0);
}

bool RayTracer::RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance) const 
{
	if (renderContext->tree.IntersectRay(ray, nearestHit))
		return nearestHit.distance < maxDistance;

	return FALSE;
}