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

#include "inputmanager.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

const float RayTracer::MAX_FRAME_TIME = 0.05f;
const uint32_t RayTracer::PIXELS_PER_JOB = 256;

RayTracer::RayTracer(Scheduler& scheduler) : Renderer(), scheduler(scheduler), whittedIntegrator(*this), monteCarloIntegrator(*this), renderingFrame(false), maxDepth(1), frameTimer(0.0f, FLT_MAX)
{

}

void RayTracer::Initialize()
{
	
}

void RayTracer::SetRenderContext(const RenderContext* context)
{
	if (context == renderContext)
		return;

	assert(renderContext != NULL && "RayTracer can't switch render contexts!"); // TODO: Handle this more gracefully

	Renderer::SetRenderContext(context);

	Buffer* frameBuffer = context->renderTarget->frameBuffer;
	uint32_t pixels = frameBuffer->width * frameBuffer->height;

	// Reserve enough memory for all pixels
	pixelList.reserve(pixels);

	// Add all the pixel coordinates to the list
	for (uint32_t y = 0; y < frameBuffer->height; ++y)
	{
		for (uint32_t x = 0; x < frameBuffer->width; ++x)
			pixelList.push_back(Point2(x, y));
	}

	std::random_shuffle(pixelList.begin(), pixelList.end());

	// Create render jobs to hold all pixels
	uint32_t renderJobCount = ceil(pixels / (float)PIXELS_PER_JOB);

	std::vector<Point2>::iterator it = pixelList.begin();
	for (uint32_t renderJobIdx = 0; renderJobIdx < renderJobCount; ++renderJobIdx)
	{
		RenderJob* job = new RenderJob(*this, it + renderJobIdx * PIXELS_PER_JOB, it + std::min((renderJobIdx + 1) * PIXELS_PER_JOB, pixels));
		renderJobs.push_back(job);
	}
}

void RayTracer::PreRender()
{
	frameTimer.Tick();

	// Schedule all render jobs
	for (auto it = renderJobs.begin(); it != renderJobs.end(); ++it)
		scheduler.ScheduleJob(*it);

	renderingFrame = true;
}

void RayTracer::PostRender()
{
	float time = frameTimer.Poll();

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

	// Write to color buffer
	frameBuffer->SetPixel(pixel[0], pixel[1], shadingInfo.color);
}

void RayTracer::CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth)
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

	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();
	CalculateShading(ray, hitInfo, *renderable->material, shadingInfo, depth);
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const Material& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();
	
	const Vector3 viewVector = -ray.direction;
	const Vector3& normal = hitInfo.normal;

	Vector3 radiance(0.0f, 0.0f, 0.0f);

	assert(material.bsdf != NULL);

	// Iterate through all the lights
	// TODO: make this functionality of the base class SurfaceIntegrator. Then each integrator can decide whether it wants direct lighting or not
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = lightData.lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 toLight;
		float distanceToLight;

		float intensity = light.intensity;

		if (light.type != LightData::DIRECTIONAL)
		{
			toLight = light.position - hitInfo.point;

			distanceToLight = toLight.length();
			toLight.normalize();

			if (light.type == LightData::SPOT)
			{
				float angleTerm = cml::dot(light.direction, -toLight);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity /= (distanceToLight * distanceToLight);
		}
		else
		{
			toLight = -light.direction;
			distanceToLight = 1000.0f; // TODO: shadow distance render context parameter?
		}

		Ray shadowRay(hitInfo.point + toLight * 1e-5f, toLight);

		RaycastHit shadowHitInfo;
		if (RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;

		float NoL = std::max(cml::dot(normal, toLight), 0.0f);
		Vector3 lightRadiance = light.color.subvector(3) * intensity;

		radiance += material.bsdf->Sample(viewVector, toLight, normal, material) * lightRadiance * NoL;
	}

	if (depth < maxDepth)
	{
		if (InputManager::Instance().GetKey('G'))
			radiance += whittedIntegrator.Li(ray, hitInfo, material, depth);
		else
			radiance += monteCarloIntegrator.Li(ray, hitInfo, material, depth);
		
		/*/
		if (FALSE)
		{
			float ior = 0.75f;

			// Refraction
			Vector3 innerRefractionDirection;
			VectorUtil<3>::Refract(ray.direction, normal, ior, innerRefractionDirection);

			Ray innerRefractionRay(hitInfo.point - normal * 0.01f, innerRefractionDirection);

			Ray refractionRay;

			RaycastHit refractionHit;
			if (RayCast(innerRefractionRay, refractionHit))
			{
				if (refractionHit.inside)
				{
					ior = 1.0f / ior;

					Vector3 outerRefractionDirection;
					VectorUtil<3>::Refract(innerRefractionDirection, refractionHit.normal, ior, outerRefractionDirection);

					refractionRay = Ray(refractionHit.point + normal * 0.01f, outerRefractionDirection);
				}
				else
				{
					refractionRay = innerRefractionRay;
				}

			}

			ShadingInfo refractionShading;
			CalculateShading(refractionRay, refractionShading, depth + 1);

			radiance += refractionShading.color.subvector(3);
		}
		/**/
	}
	
	shadingInfo.color = Color(radiance, 1.0);
}


float RayTracer::Fresnel(const Vector3& v, const Vector3& normal, float ior)
{
	float cosi = cml::dot(v, normal);
	float etai = 1, etat = ior;

	if (cosi > 0)
		std::swap(etai, etat);

	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));

	// Total internal reflection
	if (sint >= 1)
		return 1.0f;

	float cost = sqrtf(std::max(0.f, 1 - sint * sint));
	cosi = fabsf(cosi);

	float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
	float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

	return (Rs * Rs + Rp * Rp) / 2;
}


bool RayTracer::RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance)
{
	std::vector<Node*>::const_iterator it;
	
	nearestHit.distance = FLT_MAX;

	// Iterate through all nodes in the scene
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node* node = *it;
		const Renderable* renderable = node->GetComponent<Renderable>();

		if (renderable == NULL)
			continue;

		const Shape* shape = renderable->shape;

		// Perform the ray intersection		
		RaycastHit hitInfo;
		if (!shape->IntersectRay(ray, hitInfo))
			continue;

		// If the intersection is further away than a previous intersection, we're not interested
		if (hitInfo.distance > nearestHit.distance || hitInfo.distance > maxDistance)
			continue;
		
		nearestHit = hitInfo;
		nearestHit.node = node;
	}

	return nearestHit.distance < maxDistance;
}

#if FALSE
void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::const_iterator it;

	// Iterate through all nodes in the scene
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node& node = **it;

		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model->meshes.size(); ++cMesh)
		{
			const Mesh& mesh = *node.model->meshes[cMesh];
			const Material& material = *node.model->materials[cMesh];

			// Check if some part of the mesh is hit by the ray
			RaycastHit boundsHitInfo(ray);
			if (!mesh.bounds.IntersectRay(ray, boundsHitInfo))
				continue;

			// Create an inversed transformation matrix to transform to object space
			Matrix44 world2object = node.transform->WorldMtx();
			world2object.inverse();

			// Transform ray to object space to use for intersection
			Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));
			objectSpaceRay.direction.normalize();

			// Stack for nodes we still have to traverse
			std::stack<const KDTree*> nodesLeft;

			// Start with the root node of the mesh
			nodesLeft.push(&mesh.As<MeshEx>()->tree);

			while (!nodesLeft.empty())
			{
				// Get the top node from the stack
				const KDTree* tree = nodesLeft.top();
				nodesLeft.pop();
				
				// The current node is a leaf node, this means we can check its contents
				if (tree->IsLeaf())
				{
					std::vector<const Shape*>::const_iterator objectIt;

					for (objectIt = tree->objects.begin(); objectIt != tree->objects.end(); ++objectIt)
					{
						const Primitive& shape = (*objectIt)->GetShape();

						// Perform the ray-triangle intersection
						RaycastHit hitInfo(objectSpaceRay);
						if (!shape.IntersectRay(objectSpaceRay, hitInfo))
							continue;

						// Only hits outside viewing frustum
						if (hitInfo.distance > cameraDepth)
							continue;

						if (depthBuffer != NULL)
						{
							// Depth testing
							float depth = 1.0f - hitInfo.distance / cameraDepth;

							if (depthBuffer->GetPixel(screenPosition[0], screenPosition[1]) >= depth)
								continue;

							// Write to depth buffer
							depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
						}

						// Write to color buffer
						frameBuffer->SetPixel(screenPosition[0], screenPosition[1], Color::WHITE);
					}
				}
				else
				{
					// Construct the plane along which this tree node is split
					Vector3 planeNormal(0.0f, 0.0f, 0.0f); planeNormal[tree->Axis()] = 1.0f;
					Plane splitPlane(tree->SplitPoint(), planeNormal);

					// Determine which side of the plane the origin of the ray is, this side should always be visited
					int side = splitPlane.SideOfPlane(objectSpaceRay.origin);
					const KDTree *near, *far;
					
					if (side > 0)
					{
						near = tree->upperNode;
						far = tree->lowerNode;
					}
					else
					{
						near = tree->lowerNode;
						far = tree->upperNode;
					}

					// If the ray intersects the split plane, we need to visit the far node
					RaycastHit hitInfo(objectSpaceRay);
					if (splitPlane.IntersectRay(objectSpaceRay, hitInfo))
						nodesLeft.push(far);
					
					// Push the near node last so that we visit it first
					nodesLeft.push(near);
				}
			}

		}
	}
}
#endif

#if FALSE
void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::const_iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node& node = **it;
				
		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model.meshes.size(); ++cMesh)
		{
			const Mesh& mesh = *node.model.meshes[cMesh];
			const Material& material = *node.model.materials[cMesh];

			// Check if some part of the mesh is hit by the ray
			RaycastHit boundsHitInfo(ray);
			if (!mesh.bounds.IntersectRay(ray, boundsHitInfo))
				continue;

			for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
			{
				// Retrieve vertex indices for this triangle
				int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

				// Create triangle object based on the vertex data
				Triangle3D triangle(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);
				
				// Transform the triangle from model to world space
				triangle.Transform(node.transform.WorldMtx());
				triangle.PreCalculateBarycentric();

				// Perform the ray-triangle intersection
				RaycastHit hitInfo(ray);
				if (!triangle.IntersectRay(ray, hitInfo))
					continue; 

				// Only hits outside viewing frustum
				if (hitInfo.distance > cameraDepth)
					continue;
				
				// Interpolate pixel data
				SoftwareShader::VertexToPixel interpolated;
				interpolated.screenPosition = Vector4(((float)screenPosition[0]) / frameBuffer->width,
													  ((float)screenPosition[1]) / frameBuffer->height,
														hitInfo.distance / cameraDepth, 1.0f);
				interpolated.worldPosition = Vector4(hitInfo.point, 1.0f);


				if (depthBuffer != NULL)
				{
					// Depth testing
					float depth = 1.0f - interpolated.screenPosition[2];

					if (depthBuffer->GetPixel(screenPosition[0], screenPosition[1]) >= depth)
						continue;

					// Write to depth buffer
					depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
				}

				// Write to color buffer
				frameBuffer->SetPixel(screenPosition[0], screenPosition[1], Color::WHITE);
			}
		}
	}

}
#endif