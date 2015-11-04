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

using namespace AwesomeRenderer;

const float RayTracer::MAX_FRAME_TIME = 0.05f;
const int RayTracer::MAX_DEPTH = 2;

RayTracer::RayTracer() : Renderer(), pixelIdx(0), timer(0.0f, FLT_MAX), frameTimer(0.0f, FLT_MAX)
{

}

void RayTracer::Initialize()
{
	
}

void RayTracer::SetRenderContext(const RenderContext* context)
{
	Renderer::SetRenderContext(context);

	Buffer* frameBuffer = context->renderTarget->frameBuffer;
	int pixels = frameBuffer->width * frameBuffer->height;
	
	// Check if the pixel list needs to be updated
	if (pixels != pixelList.size())
	{
		pixelList.clear();

		// Reserve enough memory for al lpixels
		pixelList.reserve(pixels);

		// Add all the pixel coordinates to the list
		for (uint32_t y = 0; y < frameBuffer->height; ++y)
		{
			for (uint32_t x = 0; x < frameBuffer->width; ++x)
				pixelList.push_back(Point2(x, y));
		}

		std::random_shuffle(pixelList.begin(), pixelList.end());

		pixelIdx = 0;
	}
}

void RayTracer::PreRender()
{
	//renderContext->renderTarget->Clear(Color::BLACK, renderContext->clearFlags);

	frameTimer.Tick();
}

void RayTracer::PostRender()
{
	float time = frameTimer.Poll();

	printf("[RayTracer]: Rendered frame in %.0fms.\n", time * 1000);
}

void RayTracer::Render()
{
	// Measure the start time of our rendering process so we can limit the frame time
	const TimingInfo& start = timer.Tick();
	int pixelsDrawn = 0;

	// Check if the last frame was a complete one.
	if (pixelIdx >= pixelList.size())
	{
		PreRender();
		pixelIdx = 0;
	}

	while (timer.Poll() < MAX_FRAME_TIME)
	{
		// Get the next pixel
		const Point2& pixel = pixelList[pixelIdx];

		Render(pixel);

		++pixelsDrawn;

		// If we rendered the last pixel
		if (++pixelIdx >= pixelList.size())
		{
			PostRender();
			break;
		}
	}

	//printf("[RayTracer]: %d pixels drawn.\n", pixelsDrawn);
}

void RayTracer::Present(Window& window)
{
	GdiBuffer* buffer = static_cast<GdiBuffer*>(renderContext->renderTarget->frameBuffer);

	if (buffer != NULL)
		window.DrawBuffer(*buffer);
}

void RayTracer::Cleanup()
{

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
		if (renderContext->skybox != NULL && depth == 0)
			renderContext->skybox->Sample(ray.direction, shadingInfo.color);

		return;
	}

	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();

	// Check if this node has a PBR material
	const PbrMaterial* pbrMaterial = dynamic_cast<PbrMaterial*>(renderable->material);
	if (pbrMaterial != NULL)
	{
		CalculateShading(ray, hitInfo, *pbrMaterial, shadingInfo, depth);
		return;
	}

	// Check if this node has a phong material
	const PhongMaterial* phongMaterial = dynamic_cast<PhongMaterial*>(renderable->material);
	if (phongMaterial != NULL)
	{
		CalculateShading(ray, hitInfo, *phongMaterial, shadingInfo, depth);
		return;
	}
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PhongMaterial& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();

	Color diffuse = material.diffuseColor;
	Color specular = material.specularColor;

	Color diffuseLight = Color::BLACK;
	Color specularLight = Color::BLACK;

	float shininess = material.shininess;

	// Iterate through all the lights
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = lightData.lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 toLight;
		float intensity = light.intensity;
		
		if (light.type != LightData::DIRECTIONAL)
		{
			toLight = light.position - hitInfo.point;
			float distanceToLight = toLight.length();
			toLight.normalize();

			Ray shadowRay(hitInfo.point + toLight * 0.001f, toLight);
			RaycastHit shadowHitInfo;
			if (RayCast(shadowRay, shadowHitInfo, distanceToLight))
				continue;

			if (light.type == LightData::SPOT)
			{
				float angleTerm = cml::dot(light.direction, -toLight);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity *= 1.0f / (light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight));
		}
		else
			toLight = -light.direction;
		
		// Compute the diffuse term
		float diffuseTerm = std::max(cml::dot(hitInfo.normal, toLight), 0.0f);
		diffuseLight += light.color * diffuseTerm * intensity;

		// Compute the specular term
		if (diffuseTerm > 0.0f)
		{
			Vector3 toEye = cml::normalize(renderContext->camera->position - hitInfo.point);
			Vector3 halfVector = cml::normalize(toLight + toEye);

			float specularTerm = std::pow(std::max(cml::dot(hitInfo.normal, halfVector), 0.0f), shininess);
			specularLight += light.color * specularTerm * intensity;
		}
	}

	if (depth < MAX_DEPTH)
	{
		{
			// Reflection
			Vector3 reflectionDirection;
			VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

			Ray reflectionRay(hitInfo.point + reflectionDirection * 0.001f, reflectionDirection);

			ShadingInfo reflectionShading;
			CalculateShading(reflectionRay, reflectionShading, depth + 1);

			// Compute the diffuse term
			float diffuseTerm = std::max(cml::dot(hitInfo.normal, reflectionDirection), 0.0f);

			Vector3 toEye = cml::normalize(renderContext->camera->position - hitInfo.point);
			Vector3 halfVector = cml::normalize(reflectionDirection + toEye);

			float specularTerm = std::pow(std::max(cml::dot(hitInfo.normal, halfVector), 0.0f), shininess);
			specularLight += reflectionShading.color * specularTerm;
		}

		if (FALSE)
		{
			// Refraction
			Vector3 refractionDirection;
			VectorUtil<3>::Refract(ray.direction, hitInfo.normal, 1.02f, refractionDirection);

			Ray refractionRay(hitInfo.point + refractionDirection * 0.001f, refractionDirection);

			ShadingInfo refractionShading;
			CalculateShading(refractionRay, refractionShading, depth + 1);

			diffuseLight += refractionShading.color;
		}

	}

	shadingInfo.color = diffuse * (lightData.ambient + diffuseLight) + specular * specularLight;
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PbrMaterial& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();
	
	Vector3 viewVector = -ray.direction;
	Vector3 normal = hitInfo.normal;

	Vector3 diffuseRadiance(0.0f, 0.0f, 0.0f);
	Vector3 specularRadiance(0.0f, 0.0f, 0.0f);

	Vector3 F0 = material.specular.subvector(3);
	Vector3 ks(0.0f, 0.0f, 0.0f);

	// Iterate through all the lights
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = lightData.lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 toLight;
		float intensity = light.intensity;

		if (light.type != LightData::DIRECTIONAL)
		{
			toLight = light.position - hitInfo.point;

			float distanceToLight = toLight.length();
			toLight.normalize();

			Ray shadowRay(hitInfo.point + toLight * 0.001f, toLight);	// TODO: this bias needs to be based on something
			RaycastHit shadowHitInfo;
			if (RayCast(shadowRay, shadowHitInfo, distanceToLight))
				continue;

			if (light.type == LightData::SPOT)
			{
				float angleTerm = cml::dot(light.direction, -toLight);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity *= 1.0f / (light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight));
		}
		else
			toLight = -light.direction;

		Vector3 lightRadiance = light.color.subvector(3) * intensity;
		Vector3 specularContribution;

		diffuseRadiance += lightRadiance;
		specularRadiance += SpecularCookTorrance(lightRadiance, viewVector, normal, toLight, F0, material.roughness, specularContribution);

		ks += specularContribution;
	}

	if (depth < MAX_DEPTH)
	{
		Vector3 reflectionDirection;
		VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

		// Reflection
		Ray reflectionRay(hitInfo.point + reflectionDirection * 0.0001f, reflectionDirection);

		ShadingInfo reflectionShading;
		CalculateShading(reflectionRay, reflectionShading, depth + 1);

		Vector3 lightRadiance = reflectionShading.color.subvector(3);
		Vector3 specularContribution;

		diffuseRadiance += lightRadiance;
		specularRadiance += SpecularCookTorrance(lightRadiance, viewVector, normal, reflectionDirection, F0, material.roughness, specularContribution);

		ks += specularContribution;
	}
	
	Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
	
	Color diffuse = material.albedo * Color(diffuseRadiance * kd, 1.0f);
	Color specular = Color(specularRadiance, 1.0f);
	
	shadingInfo.color = diffuse + specular;
}

Vector3 RayTracer::SpecularCookTorrance(const Vector3& radiance, const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks)
{
	// Calculate the half vector
	Vector3 halfVector = cml::normalize(l + v);
	float cosT = Util::Clamp(cml::dot(l, n), 0.0f, 1.0f);
	float sinT = sqrt(1 - cosT * cosT);

	// Fresnel term
	Vector3 fresnel = FresnelSchlick(Util::Clamp(cml::dot(halfVector, v), 0.0f, 1.0f), F0);
	ks = fresnel;

	// Normal distribution term
	float distribution = DistributionGGX(n, halfVector, roughness);

	// Geometry term (once for shadowing and once for masking)
	float geometry = GeometryGGX(v, n, halfVector, roughness) * GeometryGGX(l, n, halfVector, roughness);

	// Calculate the Cook-Torrance denominator
	float denominator = Util::Clamp(4 * Util::Clamp(cml::dot(v, n), 0.0f, 1.0f) * Util::Clamp(cml::dot(l, n), 0.0f, 1.0f), 0.001f, 1.0f);
	
	// Apply light radiance with BRDF
	return radiance * (Vector3)((fresnel * geometry * distribution) / denominator);
}

float RayTracer::chiGGX(float v)
{
	return v > 0.0f ? 1.0f : 0.0f;
}

float RayTracer::DistributionGGX(Vector3 n, Vector3 h, float alpha)
{
	/*
	float alpha2 = alpha * alpha;
	float NoH = Util::Clamp(cml::dot(n, h), 0.0f, 1.0f);
	float denom = NoH * NoH * (alpha2 - 1.0f) + 1.0f;
	return alpha2 / ((float)PI * denom * denom);

	/*/
	float alpha2 = alpha * alpha;
	float NoH = Util::Clamp(cml::dot(n, h), 0.0f, 1.0f);
	float NoH2 = NoH * NoH;
	float den = NoH2 * alpha2 + (1 - NoH2);

	return (chiGGX(NoH) * alpha2) / ((float) PI * den * den);
	//*/
}

float RayTracer::GeometryGGX(Vector3 v, Vector3 n, Vector3 h, float alpha)
{
	/*
	float NoV = Util::Clamp(cml::dot(v, n), 0.0f, 1.0f);

	return 1.0f / (NoV * (1.0f - alpha) + alpha);

	/*/
	float VoH2 = Util::Clamp(cml::dot(v, h), 0.0f, 1.0f);
	float chi = chiGGX(VoH2 / Util::Clamp(cml::dot(v, n), 0.0f, 1.0f));
	VoH2 = VoH2 * VoH2;
	
	float tan2 = (1 - VoH2) / VoH2;

	return (chi * 2) / (1 + sqrt(1 + alpha * alpha * tan2));
	//*/
}

Vector3 RayTracer::FresnelSchlick(float cosT, Vector3 F0)
{
	return F0 + (1.0f - F0) * pow(1 - cosT, 5);
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

		const Primitive* shape = renderable->primitive;

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
					std::vector<const Object*>::const_iterator objectIt;

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