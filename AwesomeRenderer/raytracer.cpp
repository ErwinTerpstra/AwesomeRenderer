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

#include "inputmanager.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

const float RayTracer::MAX_FRAME_TIME = 0.05f;

RayTracer::RayTracer() : Renderer(), random(Random::instance), pixelIdx(0), maxDepth(1), sampleCount(16), timer(0.0f, FLT_MAX), frameTimer(0.0f, FLT_MAX)
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

		// Reserve enough memory for all pixels
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

void RayTracer::ResetFrame()
{
	PostRender();
	pixelIdx = pixelList.size(); 
	
	renderContext->renderTarget->Clear(Color::BLACK, renderContext->clearFlags);
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

	// Check if this node has a PBR material
	const PbrMaterial* pbrMaterial = renderable->material->As<PbrMaterial>();
	if (pbrMaterial != NULL)
	{
		CalculateShading(ray, hitInfo, *pbrMaterial, shadingInfo, depth);
		return;
	}
	
	// Check if this node has a phong material
	const PhongMaterial* phongMaterial = renderable->material->As<PhongMaterial>();
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

	const Vector3& normal = hitInfo.normal;

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
		float diffuseTerm = std::max(cml::dot(normal, toLight), 0.0f);
		diffuseLight += light.color * diffuseTerm * intensity;

		// Compute the specular term
		if (diffuseTerm > 0.0f)
		{
			Vector3 halfVector = cml::normalize(-ray.direction + toLight);

			float specularTerm = std::pow(std::max(cml::dot(normal, halfVector), 0.0f), shininess);
			specularLight += (light.color * specularTerm * intensity) * diffuseTerm;
		}
	}

	if (depth < maxDepth)
	{
		Color reflection;
		Color refraction;
		float ior = 0.75f;
		
		{
			// Reflection
			Vector3 reflectionDirection;
			VectorUtil<3>::Reflect(ray.direction, normal, reflectionDirection);

			Ray reflectionRay(hitInfo.point + normal * 0.01f, reflectionDirection);

			ShadingInfo reflectionShading;
			CalculateShading(reflectionRay, reflectionShading, depth + 1);

			reflection = reflectionShading.color;
		}

		if (FALSE)
		{
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

			refraction = refractionShading.color;
		}

		float fresnel = Fresnel(ray.direction, normal, ior);

		Color transmittedLight = reflection * fresnel + refraction * (1.0f - fresnel);
		specularLight += transmittedLight;
	}

	shadingInfo.color = diffuse * diffuseLight + specular * specularLight;
}

void RayTracer::CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PbrMaterial& material, ShadingInfo& shadingInfo, int depth)
{
	const LightData& lightData = *renderContext->lightData;
	const Renderable* renderable = hitInfo.node->GetComponent<Renderable>();
	
	const Vector3 viewVector = -ray.direction;
	const Vector3& normal = hitInfo.normal;

	// TODO: Evaluate ambient with diffuse BRDF
	Vector3 diffuseRadiance = (lightData.ambient * material.albedo).subvector(3);
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

		Vector3 ks;
		specularRadiance += SpecularCookTorrance(viewVector, normal, toLight, F0, material.roughness, ks) * lightRadiance * NoL;

		Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
		diffuseRadiance += DiffuseLambert(material.albedo.subvector(3)) * lightRadiance * kd * NoL;
	}

	if (depth < maxDepth)
	{
		if (InputManager::Instance().GetKey('G'))
		{
			Vector3 reflectionDirection;
			VectorUtil<3>::Reflect(ray.direction, normal, reflectionDirection);

			// Reflection
			Ray reflectionRay(hitInfo.point + normal * 1e-5f, reflectionDirection);

			ShadingInfo reflectionShading;
			CalculateShading(reflectionRay, reflectionShading, depth + 1);

			assert(fabs(cml::dot(normal, reflectionDirection) - cml::dot(normal, viewVector)) < 1e-5f);

			float NoL = Util::Clamp01(cml::dot(normal, reflectionDirection));
			Vector3 lightRadiance = reflectionShading.color.subvector(3);

			Vector3 ks;
			specularRadiance += SpecularCookTorrance(viewVector, normal, reflectionDirection, F0, material.roughness, ks) * lightRadiance * NoL;
			
			Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
			diffuseRadiance += DiffuseLambert(material.albedo.subvector(3)) * lightRadiance * kd * NoL;

		}
		else
		{
			Vector3 diffuseReflection(0.0f, 0.0f, 0.0f);
			Vector3 specularReflection(0.0f, 0.0f, 0.0f);
			
			for (int sampleIdx = 0; sampleIdx < sampleCount; ++sampleIdx)
			{
				float pdf;
				Vector3 reflectionDirection = GenerateSampleVector(viewVector, normal, material.roughness, pdf);

				if (pdf < 1e-5f)
					continue;

				// Reflection
				Ray reflectionRay(hitInfo.point + normal * 1e-5f, reflectionDirection);
				
				ShadingInfo reflectionShading;
				CalculateShading(reflectionRay, reflectionShading, depth + 1);

				float NoL = cml::dot(normal, reflectionDirection);
				assert(NoL + 1e-5f >= 0.0f && NoL - 1e-5f <= 1.0f);

				Vector3 lightRadiance = reflectionShading.color.subvector(3);

				if (lightRadiance.length_squared() <= 1e-5)
					continue;
					
				Vector3 ks;
				specularReflection += SpecularCookTorrance(viewVector, normal, reflectionDirection, F0, material.roughness, ks) * lightRadiance * NoL / pdf;
				
				Vector3 kd = (1.0f - ks) * (1.0f - material.metallic);
				diffuseReflection += DiffuseLambert(material.albedo.subvector(3)) * lightRadiance * kd * NoL / pdf;
			}

			diffuseRadiance += diffuseReflection / sampleCount;
			specularRadiance += specularReflection / sampleCount;
		}
	}
	
	Color diffuse = InputManager::Instance().GetKey('T') ? Color::BLACK : Color(diffuseRadiance, 1.0f);
	Color specular = InputManager::Instance().GetKey('Y') ? Color::BLACK : Color(specularRadiance, 1.0f);
	
	shadingInfo.color = diffuse + specular;
}


Vector3 RayTracer::GenerateSampleVector(const Vector3& v, const Vector3& n, float roughness, float& pdf)
{
	float r1 = random.NextFloat();
	float r2 = random.NextFloat();

	float phi, theta;
	ImportanceSampleGGX(Vector2(r1, r2), roughness, phi, theta);
	pdf = PDFGGX(phi, theta, roughness);
	
	float sinTheta = sinf(theta);
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);

	Vector3 sample = Vector3(x, cosf(theta), z);

	assert(VectorUtil<3>::IsNormalized(sample));

	Vector3 right, forward;
	VectorUtil<3>::OrthoNormalize(n, right, forward);

	assert(fabs(cml::dot(n, forward)) < 1e-5f);
	assert(fabs(cml::dot(n, right)) < 1e-5f);
	assert(fabs(cml::dot(right, forward)) < 1e-5f);

	Matrix33 transform(
		right[0],	right[1],	right[2],
		n[0],		n[1],		n[2],
		forward[0],	forward[1],	forward[2]
	);

	assert(cml::dot(sample, Vector3(0.0f, 1.0f, 0.0f)) >= 0.0f - 1e-5f);

	sample = transform_vector(transform, sample);
		
	assert(VectorUtil<3>::IsNormalized(sample));

	return sample;
}

void RayTracer::ImportanceSampleGGX(const Vector2& r, float alpha, float& phi, float& theta)
{
	phi = 2.0f * PI * r[0];
	theta = acos(sqrt((1.0f - r[1]) / ((alpha * alpha - 1.0f) * r[1] + 1.0f)));
}

float RayTracer::PDFGGX(float phi, float theta, float alpha)
{
	float alpha2 = alpha * alpha;
	float cosTheta = cosf(theta);
	float denom = (cosTheta * cosTheta * (alpha2 - 1.0f)) + 1.0f;

	return (alpha2 / std::max((float)PI * denom * denom, 1e-7f)) * cosTheta * sinf(theta);
}

Vector3 RayTracer::DiffuseLambert(const Vector3& albedo)
{
	return albedo / PI;
}

Vector3 RayTracer::SpecularCookTorrance(const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks)
{
	assert(VectorUtil<3>::IsNormalized(v));
	assert(VectorUtil<3>::IsNormalized(n));
	assert(VectorUtil<3>::IsNormalized(l));

	roughness = roughness * roughness;
	
	// Calculate the half vector
	Vector3 h = cml::normalize(l + v);
		
	// Fresnel term
	Vector3 fresnel = InputManager::Instance().GetKey('Z') ? F0 : FresnelSchlick(Util::Clamp01(cml::dot(h, l)), F0);
	ks = fresnel;

	float distribution, geometry;

	// Normal distribution & geometry term
	if (InputManager::Instance().GetKey('X'))
	{
		distribution = DistributionBlinn(n, h, RoughnessToShininess(roughness));
		geometry = GeometrySmith(v, l, n, h, roughness);
	}
	else
	{
		distribution = DistributionGGX(n, h, roughness);
		geometry = GeometryGGX(v, l, n, h, roughness);
	}

	if (InputManager::Instance().GetKey('C'))
		geometry = GeometryImplicit(v, l, n, h);

	distribution = std::max(distribution, 0.0f);
	geometry = std::max(geometry, 0.0f);

	// Calculate the Cook-Torrance denominator
	float denominator = std::max(4 * Util::Clamp01(cml::dot(n, v)) * Util::Clamp01(cml::dot(n, l)), 1e-7f);
	
	// Return the evaluated BRDF
	Vector3 result = ((fresnel * geometry * distribution) / denominator);
	result[0] = Util::Clamp01(result[0]);
	result[1] = Util::Clamp01(result[1]);
	result[2] = Util::Clamp01(result[2]);

	return result;
}

float RayTracer::RoughnessToShininess(float a)
{
	return std::max((2.0f / (a * a)) - 2.0f, 1e-7f);
}

float RayTracer::DistributionBlinn(const Vector3 & n, const Vector3& h, float e)
{
	float NoH = Util::Clamp01(cml::dot(n, h));

	return ((e + 2) * INV_TWO_PI) * std::pow(NoH, e);
}

float RayTracer::DistributionGGX(const Vector3& n, const Vector3& h, float alpha)
{
	float alpha2 = alpha * alpha;
	float NoH = Util::Clamp01(cml::dot(n, h));
	float denom = (NoH * NoH * (alpha2 - 1.0f)) + 1.0f;
	return alpha2 / std::max((float)PI * denom * denom, 1e-7f);
}

float RayTracer::GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h)
{
	float NoV = Util::Clamp01(cml::dot(n, v));
	float NoL = Util::Clamp01(cml::dot(n, l));

	return NoL * NoV;
}

float RayTracer::GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h)
{
	float NoH = Util::Clamp01(cml::dot(n, h));
	float NoV = Util::Clamp01(cml::dot(n, v));
	float NoL = Util::Clamp01(cml::dot(n, l));
	float VoH = Util::Clamp(cml::dot(v, h), 1e-7f, 1.0f);

	return std::min(1.0f, std::min((2.0f * NoH * NoV) / VoH,
								   (2.0f * NoH * NoL) / VoH));
}

float RayTracer::GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a)
{
	a += 1;

	return G1Schlick(l, n, a) * G1Schlick(v, n, a);
}

float RayTracer::G1Schlick(const Vector3& v, const Vector3& n, float a)
{
	float NoV = Util::Clamp01(cml::dot(v, n));
	float k = (a * a) / 8;
	return NoV / (NoV * (1.0f - k) + k);
}

float RayTracer::GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a)
{
	return G1GGX(v, n, h, a) * G1GGX(l, n, h, a);
}

float RayTracer::G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a)
{
	float HoV = cml::dot(h, v);
	
	if (HoV < 0.0f)
		return 0.0f;

	float a2 = a * a;

	float NoV2 = cml::dot(n, v);
	NoV2 = NoV2 * NoV2;

	float tan2 = (1.0f - NoV2) / NoV2;
	
	return (2.0f / std::max(1.0f + sqrt(1.0f + a2 * tan2), 1e-7f));
}

Vector3 RayTracer::FresnelSchlick(float cosT, Vector3 F0)
{
	return F0 + (1.0f - F0) * pow(1 - cosT, 5);
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