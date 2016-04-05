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
const int RayTracer::MAX_DEPTH = 5;

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
		if (renderContext->skybox != NULL)
			renderContext->skybox->Sample(ray.direction, shadingInfo.color);

		return;
	}

	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();

	//*
	
	// Check if this node has a PBR material
	const PbrMaterial* pbrMaterial = static_cast<PbrMaterial*>(renderable->material);
	if (pbrMaterial != NULL)
	{
		CalculateShading(ray, hitInfo, *pbrMaterial, shadingInfo, depth);
		return;
	}

	/*/

	// Check if this node has a phong material
	const PhongMaterial* phongMaterial = static_cast<PhongMaterial*>(renderable->material);
	if (phongMaterial != NULL)
	{
		CalculateShading(ray, hitInfo, *phongMaterial, shadingInfo, depth);
		return;
	}

	//*/
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PhongMaterial& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();

	Color diffuse = material.diffuseColor;
	Color specular = material.specularColor;

	Color diffuseLight = lightData.ambient;
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

			intensity *= 1.0f / (light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight));
		}
		else
		{
			toLight = -light.direction;
			distanceToLight = 1000.0f; // TODO: shadow distance render context parameter?
		}

		Ray shadowRay(hitInfo.point + toLight * 0.001f, toLight);
		RaycastHit shadowHitInfo;
		if (RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;
		
		// Compute the diffuse term
		float diffuseTerm = std::max(cml::dot(hitInfo.normal, toLight), 0.0f);
		diffuseLight += light.color * diffuseTerm * intensity;

		// Compute the specular term
		if (diffuseTerm > 0.0f)
		{
			Vector3 halfVector = cml::normalize(-ray.direction + toLight);

			float specularTerm = std::pow(std::max(cml::dot(hitInfo.normal, halfVector), 0.0f), shininess);
			specularLight += (light.color * specularTerm * intensity) * diffuseTerm;
		}
	}

	if (depth < MAX_DEPTH)
	{
		Color reflection;
		Color refraction;
		float ior = 0.75f;
		
		{
			// Reflection
			Vector3 reflectionDirection;
			VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

			Ray reflectionRay(hitInfo.point + hitInfo.normal * 0.01f, reflectionDirection);

			ShadingInfo reflectionShading;
			CalculateShading(reflectionRay, reflectionShading, depth + 1);

			reflection = reflectionShading.color;
		}

		if (FALSE)
		{
			// Refraction
			Vector3 innerRefractionDirection;
			VectorUtil<3>::Refract(ray.direction, hitInfo.normal, ior, innerRefractionDirection);
			
			Ray innerRefractionRay(hitInfo.point - hitInfo.normal * 0.01f, innerRefractionDirection);

			Ray refractionRay;

			RaycastHit refractionHit;
			if (RayCast(innerRefractionRay, refractionHit))
			{
				if (refractionHit.inside)
				{
					ior = 1.0f / ior;

					Vector3 outerRefractionDirection;
					VectorUtil<3>::Refract(innerRefractionDirection, refractionHit.normal, ior, outerRefractionDirection);

					refractionRay = Ray(refractionHit.point + hitInfo.normal * 0.01f, outerRefractionDirection);
				}
				else
				{
					refractionRay = innerRefractionRay;
				}

			}

			ShadingInfo refractionShading;
			CalculateShading(refractionRay, refractionShading, depth + 1);

			refraction = refractionShading.color;
		}

		float fresnel = Fresnel(ray.direction, hitInfo.normal, ior);

		Color transmittedLight = reflection * fresnel + refraction * (1.0f - fresnel);
		specularLight += transmittedLight;
	}

	shadingInfo.color = diffuse * diffuseLight + specular * specularLight;
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PbrMaterial& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();
	
	Vector3 viewVector = -ray.direction;
	Vector3 normal = hitInfo.normal;

	Vector3 radiance(0.0f, 0.0f, 0.0f);
	Vector3 diffuseRadiance(0.0f, 0.0f, 0.0f);
	Vector3 specularRadiance(0.0f, 0.0f, 0.0f);

	Vector3 F0 = material.specular.subvector(3);
	
	// Iterate through all the lights
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

			intensity *= 1.0f / (light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight));
		}
		else
		{
			toLight = -light.direction;
			distanceToLight = 1000.0f; // TODO: shadow distance render context parameter?
		}

		Ray shadowRay(hitInfo.point + toLight * 0.001f, toLight);
		RaycastHit shadowHitInfo;
		if (RayCast(shadowRay, shadowHitInfo, distanceToLight))
			continue;

		float NoL = std::max(cml::dot(normal, toLight), 0.0f);
		Vector3 lightRadiance = light.color.subvector(3) * intensity * NoL;

		Vector3 ks;
		specularRadiance += SpecularCookTorrance(lightRadiance, viewVector, normal, toLight, F0, material.roughness, ks);

		Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
		diffuseRadiance += DiffuseLambert(lightRadiance, hitInfo.normal, toLight);
	}

	if (depth < MAX_DEPTH)
	{
		Vector3 reflectionDirection;
		VectorUtil<3>::Reflect(ray.direction, hitInfo.normal, reflectionDirection);

		// Reflection
		Ray reflectionRay(hitInfo.point + hitInfo.normal * 0.01f, reflectionDirection);

		ShadingInfo reflectionShading;
		CalculateShading(reflectionRay, reflectionShading, depth + 1);

		float NoL = std::max(cml::dot(normal, reflectionDirection), 0.0f);
		Vector3 lightRadiance = reflectionShading.color.subvector(3) * NoL;

		Vector3 ks;
		specularRadiance += SpecularCookTorrance(lightRadiance, viewVector, normal, reflectionDirection, F0, material.roughness, ks);

		Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
		diffuseRadiance += DiffuseLambert(lightRadiance, hitInfo.normal, reflectionDirection);
	}
	
	Color diffuse = material.albedo * Color(diffuseRadiance, 1.0f);
	Color specular = Color(specularRadiance, 1.0f);
	
	shadingInfo.color = diffuse + specular;
}

Vector3 RayTracer::DiffuseLambert(const Vector3& radiance, const Vector3& n, const Vector3& l)
{
	return radiance;
}

Vector3 RayTracer::SpecularCookTorrance(const Vector3& radiance, const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks)
{
	// Calculate the half vector
	Vector3 h = cml::normalize(l + v);

	float cosT = Util::Clamp01(cml::dot(l, n));
	float sinT = sqrt(1 - cosT * cosT);
	
	/*
	// GGX D and G term

	// Normal distribution term
	float distribution = DistributionGGX(n, halfVector, roughness);

	// Geometry term (once for shadowing and once for masking)
	float geometry = GeometryGGX(v, n, halfVector, roughness) * GeometryGGX(l, n, halfVector, roughness);
	*/

	// Fresnel term
	Vector3 fresnel = FresnelSchlick(Util::Clamp01(cml::dot(h, v)), F0);

	// Normal distribution term
	float distribution = DistributionBlinn(n, h, 1.0f);
	
	// Geometry term
	float geometry = Geometry(v, l, n, h);

	// Calculate the Cook-Torrance denominator
	float denominator = Util::Clamp01(4 * abs(cml::dot(v, n)) * abs(cml::dot(l, n)));

	ks = fresnel;

	// Apply light radiance with BRDF
	return radiance * (Vector3)((fresnel * geometry * distribution) / denominator);
}

float RayTracer::DistributionBlinn(const Vector3 & n, const Vector3& h, float e)
{
	float NoH = abs(cml::dot(n, h));

	return (e + 2) * INV_TWO_PI * std::pow(NoH, e);
}

float RayTracer::DistributionGGX(const Vector3& n, const Vector3& h, float alpha)
{
	float alpha2 = alpha * alpha;
	float NoH = Util::Clamp01(cml::dot(n, h));
	float denom = NoH * NoH * (alpha2 - 1.0f) + 1.0f;
	return alpha2 / ((float)PI * denom * denom);
}

float RayTracer::Geometry(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h)
{
	float NoH = abs(cml::dot(n, h));
	float NoV = abs(cml::dot(n, v));
	float NoL = abs(cml::dot(n, l));
	float VoH = abs(cml::dot(v, h));

	return std::min(1.0f, std::min(2.0f * NoH * NoV / VoH,
								   2.0f * NoH * NoL / VoH));
}


float RayTracer::GeometryGGX(const Vector3& v, const Vector3& n, const Vector3& h, float alpha)
{
	float NoV = Util::Clamp(cml::dot(v, n), 0.0f, 1.0f);

	return 1.0f / (NoV * (1.0f - alpha) + alpha);
}

Vector3 RayTracer::FresnelSchlick(float cosT, Vector3 F0)
{
	return F0 + pow(1 - cosT, 5) * (1.0f - F0);
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