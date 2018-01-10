
#include "setup.h"

#include "awesomerenderer.h"
#include "context.h"

// Primitives
#include "transformation.h"
#include "shape.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "triangle.h"
#include "meshtriangle.h"
#include "triangle2d.h"
#include "quad.h"
#include "node.h"

// Rendering
#include "texture.h"
#include "sampler.h"

#include "material.h"
#include "phongmaterial.h"
#include "microfacetmaterial.h"
#include "bsdf.h"
#include "microfacetspecular.h"

#include "mesh.h"
#include "model.h"

#include "textmesh.h"

#include "meshex.h"
#include "modelex.h"

#include "camera.h"

#include "renderable.h"
#include "arealight.h"

// Renderer
#include "lightdata.h"
#include "skybox.h"
#include "coloredskybox.h"
#include "sixsidedskybox.h"

#include "shader.h"
#include "softwareshader.h"
#include "unlitshader.h"
#include "phongshader.h"
#include "rendercontext.h"

// Assets
#include "texturefactory.h"
#include "objloader.h"

#include "random.h"

using namespace AwesomeRenderer;

Setup::Setup(Context& context, bool calculateExtendedMeshData) : context(context), calculateExtendedMeshData(calculateExtendedMeshData)
{

}

void Setup::SetupScene()
{
	// Shader
	PhongShader* phongShader = new PhongShader();
	UnlitShader* unlitShader = new UnlitShader();

	if (FALSE)
	{
		Node* node = new Node();

		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL | Mesh::VERTEX_TEXCOORD));
		mesh->AddQuad(Vector3(1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, -1.0f));

		float uvScale = 5.0f;
		mesh->texcoords[0] = Vector2(1.0f, 1.0f) * uvScale;
		mesh->texcoords[1] = Vector2(0.0f, 1.0f) * uvScale;
		mesh->texcoords[2] = Vector2(0.0f, 0.0f) * uvScale;

		mesh->texcoords[3] = Vector2(0.0f, 0.0f) * uvScale;
		mesh->texcoords[4] = Vector2(1.0f, 0.0f) * uvScale;
		mesh->texcoords[5] = Vector2(1.0f, 1.0f) * uvScale;

		mesh->CalculateBounds();

		Texture* texture = NULL;
		context.textureFactory->GetAsset("../Assets/tiles.bmp", &texture);

		Sampler* sampler = new Sampler();
		sampler->texture = texture;

		PhongMaterial* material = new PhongMaterial(*(new Material()));
		material->provider.shader = phongShader;
		material->diffuseMap = sampler;
		material->specularColor = Color::WHITE * 0.1f;
		material->shininess = 1.0f;

		Model* model = new Model();
		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();

		node->AddComponent(model);

		Transformation* transform = new Transformation();
		transform->SetScale(Vector3(5.0f, 5.0f, 5.0f));
		node->AddComponent(transform);

		context.mainContext->nodes.push_back(node);
	}
}

void Setup::SetupLighting()
{
	context.mainContext->lightData->numPixelLights = 8;
	context.mainContext->lightData->ambient = Color::BLACK;
}

void Setup::SetupCornellBox()
{
	// CAMERA
	const Vector3 cameraPosition = Vector3(0.0f, 0.5f, -1.5f);
	context.mainCamera->SetLookAt(cameraPosition, cameraPosition + Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));
	
	context.mainCamera->focalDistance = 0.8f;
	context.mainCamera->apertureSize = 0.008f;

	// LIGHTING
	LightData::Light& light = context.mainContext->lightData->lights[0];
	
	context.mainContext->lightData->shadowDistance = 0.5f;

	light.type = LightData::LightType::POINT;
	light.position = Vector3(0.0f, 0.9f, 0.8f);
	light.color = Color::WHITE;
	light.constantAttenuation = 1.0f;
	light.quadricAttenuation = 0.1f;
	light.intensity = 2.0f;

	light.enabled = true;

	// SKYBOX
	SixSidedSkybox* skybox = new SixSidedSkybox();
	skybox->right = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0001.png");
	skybox->front = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0002.png");
	skybox->top = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0003.png");
	skybox->left = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0004.png");
	skybox->back = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0005.png");
	skybox->bottom = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0006.png");

	//context.mainContext->skybox = skybox;


	// GEOMETRY
	const bool showBox = true;
	const bool showLight = false;
	const bool showSpheres = true;
	const bool showBunny = false;

	const Color wallWhite = Color(0.725f, 0.71f, 0.68f);
	const Color wallSpecular = Color::WHITE * 0.01f;
	const float wallRoughness = 0.8f;

	// Metal
	//*
	const Color sphereDiffuse = Color::BLACK;
	const Color sphereSpecular = Color::WHITE * 0.8f;
	//const Color sphereSpecular = Color(245, 215, 121); // GOLD
	const float sphereRoughness = 0.3f;
	const float sphereMetallic = 1;
	const float sphereIor = 1.0f;
	const bool sphereTranslucent = false;
	//*/

	// Plastic
	/*
	const Color sphereDiffuse(0.8f, 0.8f, 0.8f);
	const Color sphereSpecular(0.1f, 0.1f, 0.1f);
	const float sphereRoughness = 0.6f;
	const float sphereMetallic = 0;
	const float sphereIor = 1.0f;
	const bool sphereTranslucent = false;
	//*/

	// Glass
	/*
	const Color sphereDiffuse(0.5f, 0.5f, 0.5f);
	const Color sphereSpecular(0.1f, 0.1f, 0.1f);
	const float sphereRoughness = 0.1f;
	const float sphereMetallic = 0;
	const float sphereIor = 1.4f;
	const bool sphereTranslucent = true;
	//*/

	if (showBox)
	{
		// Left wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(-0.5f, 0.5f, 0.5f));
		transform->SetRotation(QuaternionUtil::AngleAxis(-HALF_PI, Vector3(0.0f, 0.0f, 1.0f)));
		transform->SetScale(Vector3(2.0f, 2.0f, 2.0f));
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
		material->albedo = Color(0.63f, 0.0065f, 0.05f);
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = Quad::CreateUnitQuad();
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
	}

	if (showBox)
	{
		// Right wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.5f, 0.5f, 0.5f));
		transform->SetRotation(QuaternionUtil::AngleAxis(HALF_PI, Vector3(0.0f, 0.0f, 1.0f)));
		transform->SetScale(Vector3(2.0f, 2.0f, 2.0f));
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
		material->albedo = Color(0.14f, 0.45f, 0.091f);
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = Quad::CreateUnitQuad();
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
	}

	if (showBox)
	{
		// Floor
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 0.0f, 0.5f));
		transform->SetScale(Vector3(2.0f, 2.0f, 2.0f));
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = Quad::CreateUnitQuad();
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
	}

	if (showBox)
	{
		// Ceiling
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 1.0f, 0.5f));
		transform->SetRotation(QuaternionUtil::AngleAxis(PI, Vector3(1.0f, 0.0f, 0.0f)));
		transform->SetScale(Vector3(2.0f, 2.0f, 2.0f));
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = Quad::CreateUnitQuad();
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
	}

	if (showBox)
	{
		// Back wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 0.5f, 1.0f));
		transform->SetRotation(QuaternionUtil::AngleAxis(-HALF_PI, Vector3(1.0f, 0.0f, 0.0f)));
		transform->SetScale(Vector3(2.0f, 2.0f, 2.0f));
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = Quad::CreateUnitQuad();
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
	}

	if (showLight)
	{
		// Light
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 1.0f, 0.0f));
		node->AddComponent(transform);

		Material* material = new Material();
		material->emission = Color::WHITE;
		material->emissionIntensity = 80.0f;

		/*
		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL));
		mesh->AddQuad(Vector3(0.5f, 0.0f, -0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, -0.5f));
		mesh->CalculateBounds();

		Model* model = new Model();
		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();
		node->AddComponent(model);

		ModelEx* modelEx = new ModelEx(*model);		

		Renderable* renderable = new Renderable();
		renderable->shape = modelEx->meshes[0];
		renderable->material = &material->provider;
		*/

		AreaLight* areaLight = new AreaLight();
		areaLight->primitive = new Sphere(Vector3(0.0f, 0.0f, 0.0f), 0.05f);
		areaLight->material = material;

		node->AddComponent(areaLight);

		context.mainContext->nodes.push_back(node);
		context.mainContext->lightData->areaLights.push_back(areaLight);
	}

	if (showSpheres)
	{

		MicrofacetMaterial* material = new MicrofacetMaterial(sphereMetallic > 0 ? MicrofacetMaterial::metallicBSDF : MicrofacetMaterial::dielectricBSDF, *(new Material()));
		material->albedo = sphereDiffuse;
		material->specular = sphereSpecular;
		material->metallic = sphereMetallic;
		material->roughness = sphereRoughness;
		material->provider.ior = sphereIor;
		material->provider.translucent = sphereTranslucent;

		{
			// Left sphere
			Node* node = new Node();

			Transformation* transform = new Transformation();
			transform->SetPosition(Vector3(-0.25f, 0.0f, 0.65f));
			node->AddComponent(transform);

			Renderable* renderable = new Renderable();
			renderable->shape = new Sphere(Vector3(0.0f, 0.15f, 0.0f), 0.15f);
			//renderable->shape = new AABB(Vector3(-0.15, 0.0f, -0.15f), Vector3(0.15f, 0.3f, 0.15f));
			renderable->material = &material->provider;

			node->AddComponent(renderable);

			context.mainContext->nodes.push_back(node);
		}

		{
			// right sphere
			Node* node = new Node();

			Transformation* transform = new Transformation();
			transform->SetPosition(Vector3(0.15f, 0.0f, 0.25f));
			node->AddComponent(transform);

			Renderable* renderable = new Renderable();
			renderable->shape = new Sphere(Vector3(0.0f, 0.18f, 0.0f), 0.18f);
			renderable->material = &material->provider;

			node->AddComponent(renderable);

			context.mainContext->nodes.push_back(node);
		}
	}

	if (showBunny)
	{
		Node* node = new Node();

		Quaternion q;
		cml::quaternion_rotation_axis_angle(q, Vector3(0.0f, 1.0f, 0.0f), (float) PI);

		Transformation* transform = new Transformation();
		transform->SetRotation(q);
		node->AddComponent(transform);

		MicrofacetMaterial* material = new MicrofacetMaterial(sphereMetallic > 0 ? MicrofacetMaterial::metallicBSDF : MicrofacetMaterial::dielectricBSDF, *(new Material()));
		material->albedo = sphereDiffuse;
		material->specular = sphereSpecular;
		material->metallic = sphereMetallic;
		material->roughness = sphereRoughness;
		material->provider.ior = sphereIor;
		material->provider.translucent = sphereTranslucent;


		Model* model = new Model();
		/*
		context.objLoader->Load("../Assets/bunny_lowpoly.obj", *model);
		transform->SetPosition(Vector3(-0.08f, -0.14f, 0.5f));
		transform->SetScale(Vector3(4.0f, 4.0f, 4.0f));
		/*/
		transform->SetPosition(Vector3(0.0f, 0.05f, 0.5f));
		transform->SetScale(Vector3(0.2f, 0.2f, 0.2f));
		context.objLoader->Load("../Assets/bunny.obj", *model);
		//*/

		/*
		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL));
		mesh->CreateCube(Vector3(0.0f, 0.25f, 0.0f), 0.25f);
		mesh->CalculateBounds();

		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();
		*/

		node->AddComponent(model);

		ModelEx* modelEx = new ModelEx(*model);

		for (uint32_t meshIdx = 0; meshIdx < modelEx->meshes.size(); ++meshIdx)
		{
			Node* meshNode = new Node();

			meshNode->AddComponent(transform);

			Renderable* renderable = new Renderable();
			renderable->shape = modelEx->meshes[meshIdx];
			renderable->material = &material->provider;

			meshNode->AddComponent(renderable);

			context.mainContext->nodes.push_back(meshNode);
		}

		context.mainContext->nodes.push_back(node);
	}
}

void Setup::SetupSpheres()
{
	// CAMERA
	//const Vector3 cameraPosition = Vector3(3.2f, 1.8f, 9.0f);
	const Vector3 cameraPosition = Vector3(9.0f, 1.8f, 3.2f);
	context.mainCamera->SetLookAt(cameraPosition, cameraPosition - Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0));

	// LIGHT
	LightData::Light& light = context.mainContext->lightData->lights[0];
	light.type = LightData::LightType::DIRECTIONAL;
	light.direction = Vector3(-0.5f, -0.8f, -0.5f);
	light.direction.normalize();
	light.intensity = 1.0f;
	light.color = Color(255, 244, 214);

	light.enabled = true;


	// SKYBOX

	/*
	ColoredSkybox skybox;
	skybox.top = Color(35, 71, 189) * 0.5f;
	skybox.bottom = Color(107, 205, 209) * 0.5f;
	*/

	SixSidedSkybox* skybox = new SixSidedSkybox();
	skybox->right = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0001.png");
	skybox->front = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0002.png");
	skybox->top = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0003.png");
	skybox->left = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0004.png");
	skybox->back = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0005.png");
	skybox->bottom = context.textureFactory->GetTexture("../Assets/Skyboxes/sun5deg/skyrender0006.png");

	context.mainContext->skybox = skybox;

	// GEOMETRY
	const float MIN_ROUGHNESS = 0.1f;
	const float SPHERE_RADIUS = 0.6f;
	const float SPHERE_SPACING = 0.4f;
	const uint32_t SPHERES_PER_ROW = 5;
	const Vector3 origin(0.0f, 0.5f + SPHERE_RADIUS, 0.0f);

	for (uint32_t row = 0; row < 2; ++row)
	{
		for (uint32_t sphereIdx = 0; sphereIdx < SPHERES_PER_ROW; ++sphereIdx)
		{
			Node* node = new Node();

			Vector3 position = origin;
			position[2] += sphereIdx * (SPHERE_RADIUS * 2 + SPHERE_SPACING);
			position[1] += row * (SPHERE_RADIUS * 2 + SPHERE_SPACING);

			Transformation* transform = new Transformation();
			transform->SetPosition(position);
			node->AddComponent(transform);

			MicrofacetMaterial* material = new MicrofacetMaterial(*(new Material()));
			material->roughness = MIN_ROUGHNESS + (sphereIdx / (float)(SPHERES_PER_ROW - 1.0f)) * (1.0f - MIN_ROUGHNESS);

			if (row == 0)
			{
				material->albedo = Color::WHITE * 0.5f;
				material->specular = Color::WHITE * 0.1f;
				material->metallic = 0;
			}
			else if (row == 1)
			{
				material->albedo = Color::BLACK;
				material->specular = Color::WHITE * 0.6f;
				material->metallic = 1;
			}

			Renderable* renderable = new Renderable();
			renderable->shape = new Sphere(Vector3(0.0f, 0.0f, 0.0f), SPHERE_RADIUS);

			renderable->material = &material->provider;

			node->AddComponent(renderable);

			context.mainContext->nodes.push_back(node);
		}
	}
}

void Setup::SetupSponza()
{
	// CAMERA
	const Vector3 cameraPosition = Vector3(-124.449127f, 22.0583744f, -3.47746253f);
	const Vector3 cameraLookAt = Vector3(-123.449951f, 22.0319462f, -3.44655204f);
	context.mainCamera->SetLookAt(cameraPosition, cameraLookAt, Vector3(0.0f, 1.0f, 0.0));

	// Focus end of hallway
	//context.mainCamera->focalDistance = 180.0f;
	//context.mainCamera->apertureSize = 0.8f;

	// Focus halfway hallway
	//context.mainCamera->focalDistance = 100.0f;
	//context.mainCamera->apertureSize = 0.30f;

	// Focus ground from balcony
	//context.mainCamera->focalDistance = 200.0f;
	//context.mainCamera->apertureSize = 0.45f;

	// Focus balcony corridor
	context.mainCamera->focalDistance = 50.0f;
	context.mainCamera->apertureSize = 0.4f;
	
	// LIGHT
	LightData::Light& light = context.mainContext->lightData->lights[0];
	light.type = LightData::LightType::DIRECTIONAL;
	light.direction = VectorUtil<3>::Normalize(Vector3(0, -1.0f, -0.3f));
	light.position = Vector3(0.0f, 50.0f, 0.0f);
	light.intensity = 8.0f;
	light.quadricAttenuation = 0.0001f;
	light.color = Color(255, 244, 214);

	light.enabled = false;

	{
		// Area light
		Node* node = new Node();
		
		// Center of hallway
		//Vector3 position(-80.0f, 140.0f, 15.0f);
		//float size = 6.0f;
		//float intensity = 600.0f;
		
		// High in corner
		//Vector3 position(-80.0f, 140.0f, 15.0f);
		//float size = 6.0f;
		//float intensity = 600.0f;

		// Sun
		Vector3 position(1000.0f, 3000.0f, 1500.0f);
		float size = 100.0f;
		float intensity = 100000.0f;

		Transformation* transform = new Transformation();
		transform->SetPosition(position); 
		node->AddComponent(transform);

		Material* material = new Material();
		material->emission = Color(255, 244, 214);
		material->emissionIntensity = intensity;

		AreaLight* areaLight = new AreaLight();
		areaLight->primitive = new Sphere(Vector3(0.0f, 0.0f, 0.0f), size);
		areaLight->material = material;
		node->AddComponent(areaLight);

		PhongMaterial* phongMaterial = new PhongMaterial(*(new Material()));
		phongMaterial->diffuseColor = Color::WHITE;

		Renderable* renderable = new Renderable();
		renderable->shape = areaLight->primitive;
		renderable->material = &phongMaterial->provider;
		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
		context.mainContext->lightData->areaLights.push_back(areaLight);
	}

	// SKYBOX
	SixSidedSkybox* skybox = new SixSidedSkybox();
	skybox->right = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0001.png");
	skybox->front = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0002.png");
	skybox->top = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0003.png");
	skybox->left = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0004.png");
	skybox->back = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0005.png");
	skybox->bottom = context.textureFactory->GetTexture("../Assets/Skyboxes/sun25deg/skyrender0006.png");

	context.mainContext->skybox = skybox;

	{
		// MODEL
		Node* node = new Node();
		Model* model = new Model();

		Transformation* transform = new Transformation();
		transform->SetScale(Vector3(0.1f, 0.1f, 0.1f));

		node->AddComponent(model);
		node->AddComponent(transform);

		context.objLoader->Load("../Assets/CrytekSponza/sponza.obj", *model);

		if (calculateExtendedMeshData)
		{
			ModelEx* modelEx = new ModelEx(*model);
			for (uint32_t meshIdx = 0; meshIdx < modelEx->meshes.size(); ++meshIdx)
			{
				Node* meshNode = new Node();

				meshNode->AddComponent(transform);

				Renderable* renderable = new Renderable();
				renderable->shape = modelEx->meshes[meshIdx];
				renderable->material = model->materials[meshIdx];

				meshNode->AddComponent(renderable);

				context.mainContext->nodes.push_back(meshNode);
			}
		}

		context.mainContext->nodes.push_back(node);
	}
}

void Mandelbulb(const Vector3& v, const Vector3& c, int n, Vector3& result)
{
	float r = v.length();
	float theta = atan2(v[1], sqrt(v[0] * v[0] + v[2] + v[2]));
	float phi = atan2(v[2], v[0]);

	float rn = pow(r, n);

	result = c;
	result[0] += rn * cosf(n * phi) * cosf(n * theta);
	result[2] += rn * sinf(n * phi) * cosf(n * theta);
	result[1] += rn * sinf(n * theta);
}

void Setup::SetupFractal()
{
	// CAMERA
	const Vector3 cameraPosition = Vector3(3.2f, 1.8f, 9.0f);
	context.mainCamera->SetLookAt(cameraPosition, cameraPosition - Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));
	
	MicrofacetMaterial* defaultMaterial = new MicrofacetMaterial(*(new Material()));
	defaultMaterial->roughness = 0.0f;
	defaultMaterial->albedo = Color::WHITE * 0.6f;
	defaultMaterial->specular = Color::BLACK;
	defaultMaterial->metallic = 0;
	defaultMaterial->provider.emission = Color::WHITE * 0.05f;

	MicrofacetMaterial* emissiveMaterial = new MicrofacetMaterial(*(new Material()));
	emissiveMaterial->roughness = 0.0f;
	emissiveMaterial->albedo = Color::BLACK;
	emissiveMaterial->provider.emission = Color::GREEN * 0.8f;
	emissiveMaterial->specular = Color::BLACK;
	emissiveMaterial->metallic = 0;

	const float SPHERE_RADIUS = 0.6f;
	const uint32_t MAX_ITERATIONS = 256;
	const float EMISSION_CHANCE = 0.1f;
	const int SIZE = 5;
	const float SCALE = 0.2f;
	const int POWER = 8;
	const Vector3 origin(0.0f, 0.0f, 0.0f);

	Random r;

	for (int x = -SIZE; x < SIZE; ++x)
	{
		for (int y = -SIZE; y < SIZE; ++y)
		{
			for (int z = -SIZE; z < SIZE; ++z)
			{
				Node* node = new Node();

				Vector3 p((float) x, (float)y, (float)z);
				Vector3 v(0.0f, 0.0f, 0.0f);

				uint32_t iteration;
				for (iteration = 0; iteration < MAX_ITERATIONS; ++iteration)
				{
					Mandelbulb(v, p * SCALE, POWER, v);

					if (v.length_squared() > 2)
						break;
				}

				if (iteration < MAX_ITERATIONS)
					continue;
				
				Vector3 position = origin + p * SPHERE_RADIUS * 2;

				Transformation* transform = new Transformation();
				transform->SetPosition(position);
				node->AddComponent(transform);

				Renderable* renderable = new Renderable();
				renderable->shape = new Sphere(Vector3(0.0f, 0.0f, 0.0f), SPHERE_RADIUS);

				if (r.NextFloat() < EMISSION_CHANCE)
					renderable->material = &emissiveMaterial->provider;
				else
					renderable->material = &defaultMaterial->provider;

				node->AddComponent(renderable);

				context.mainContext->nodes.push_back(node);
			}
		}
	}

}