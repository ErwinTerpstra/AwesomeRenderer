#include "stdafx.h"
#include "node.h"
#include "raytracer.h"
#include "ray.h"
#include "raycasthit.h"
#include "buffer.h"
#include "texture.h"
#include "camera.h"
#include "gdibufferallocator.h"
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
const uint32_t RayTracer::TILE_SIZE = 16;

RayTracer::RayTracer(Scheduler& scheduler) : Renderer(), 
	debugIntegrator(*this), whittedIntegrator(*this), monteCarloIntegrator(*this), renderingFrame(false), random(Random::instance),
	maxDepth(0), samplesPerPixel(1), renderedSamples(0), frameTimer(0.0f, FLT_MAX), debugPixel(-1, -1)
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

	Texture* frameBuffer = context->renderTarget->frameBuffer;
	
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
	renderedSamples += samplesPerPixel;

	printf("[RayTracer]: Rendered frame in %.0fms, total samples rendered: %u.\n", time * 1000, renderedSamples);
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
	Buffer* buffer = renderContext->renderTarget->frameBuffer;
	const GDIBufferAllocator& allocator = static_cast<const GDIBufferAllocator&>(buffer->GetAllocator());

	if (buffer != NULL)
		window.DrawBuffer(*buffer, allocator);
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

	renderedSamples = 0;
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

	Texture* frameBuffer = renderContext->renderTarget->frameBuffer;

	Color color;
	frameBuffer->GetPixel(pixel[0], pixel[1], color);

	color *= renderedSamples;

	for (uint32_t sample = 0; sample < samplesPerPixel; ++sample)
	{
		Vector2 subPixel = pixel;
		subPixel[0] += -0.5f + random.NextFloat();
		subPixel[1] += -0.5f + random.NextFloat();

		// Create a ray from the camera near plane through this pixel
		Ray primaryRay;
		renderContext->camera->ViewportToRay(pixel, primaryRay);

		ShadingInfo shadingInfo;
		CalculateShading(primaryRay, shadingInfo);

		color += shadingInfo.color;
	}

	color *= (1.0f / (renderedSamples + samplesPerPixel));
	color[3] = 1.0f;

	// Write to color buffer
	frameBuffer->SetPixel(pixel[0], pixel[1], color);
}

void RayTracer::BreakOnDebugPixel(const Point2& pixel)
{
	if (pixel == debugPixel)
	{
		debugPixel = Point2(-1, -1);
		Debug::Break();
	}

}

bool RayTracer::CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth) const
{
	const LightData& lightData = *renderContext->lightData;

	// Perform the raycast to find out which node we've hit
	if (!RayCast(ray, shadingInfo.hitInfo))
	{
		if (renderContext->skybox != NULL)
			renderContext->skybox->Sample(ray.direction, shadingInfo.color);

		return FALSE;
	}

	const Renderable* renderable = dynamic_cast<const Renderable*>(shadingInfo.hitInfo.element);
	const Material* material = renderable->material;
		
	shadingInfo.color = Color(currentIntegrator->Li(ray, shadingInfo.hitInfo, *material, *renderContext, depth), 1.0);
	return TRUE;
}

bool RayTracer::RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance) const 
{
	return renderContext->tree.IntersectRay(ray, nearestHit, maxDistance);
}