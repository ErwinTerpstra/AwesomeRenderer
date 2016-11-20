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
const uint32_t RayTracer::TILE_SIZE = 32;

RayTracer::RayTracer(Scheduler& scheduler) : Renderer(), 
	debugIntegrator(*this), whittedIntegrator(*this), monteCarloIntegrator(*this), renderingFrame(false), 
	maxDepth(0), frameTimer(0.0f, FLT_MAX), debugPixel(-1, -1)
{
	currentIntegrator = &debugIntegrator;
	
	jobGroup = scheduler.CreateJobGroup(4);
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
	
	uint32_t horizontalTiles = ceil(frameBuffer->width / (float)TILE_SIZE);
	uint32_t verticalTiles = ceil(frameBuffer->height / (float)TILE_SIZE);

	for (uint32_t verticalTile = 0; verticalTile < verticalTiles; ++verticalTile)
	{
		uint32_t y = verticalTile * TILE_SIZE;

		for (uint32_t horizontalTile = 0; horizontalTile < horizontalTiles; ++horizontalTile)
		{
			uint32_t x = horizontalTile * TILE_SIZE;

			RenderJob* job = new RenderJob(*this, x, y, std::min(TILE_SIZE, frameBuffer->width - x), std::min(TILE_SIZE, frameBuffer->height - y));
			renderJobs.push_back(job);
		}
	}
	
	std::random_shuffle(renderJobs.begin(), renderJobs.end());
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

void RayTracer::ResetFrame(bool startNewFrame)
{
	PostRender();

	renderContext->renderTarget->Clear(Color::BLACK, renderContext->clearFlags);

	if (startNewFrame)
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
	BreakOnDebugPixel(pixel);

	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;

	// Create a ray from the camera near plane through this pixel
	Ray primaryRay;
	renderContext->camera->ViewportToRay(pixel, primaryRay);

	ShadingInfo shadingInfo;
	CalculateShading(primaryRay, shadingInfo);

	// Write to color buffer
	frameBuffer->SetPixel(pixel[0], pixel[1], shadingInfo.color);
}

void RayTracer::BreakOnDebugPixel(const Point2& pixel)
{
	if (pixel == debugPixel)
	{
		debugPixel = Point2(-1, -1);
		Debug::Break();
	}

}

void RayTracer::CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth) const
{
	const LightData& lightData = *renderContext->lightData;

	// Perform the raycast to find out which node we've hit
	RaycastHit hitInfo;
	//if (depth > 0 || !RayCast(ray, hitInfo))
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
		return nearestHit.distance <= maxDistance;

	return FALSE;
}