#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "awesomerenderer.h"

#include "buffer.h"
#include "memorybuffer.h"

// Utility
#include "util.h"
#include "timer.h"
#include "transformation.h"
#include "ray.h"
#include "raycasthit.h"
#include "camera.h"

// Mult-threading
#include "threading.h"

// Primitives
#include "object.h"
#include "primitive.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "triangle.h"
#include "triangle3d.h"
#include "triangle2d.h"

// Input
#include "inputmanager.h"
#include "cameracontroller.h"

// Scene graph
#include "node.h"
#include "kdtree.h"
#include "octree.h"

// Rendering
#include "texture.h"
#include "sampler.h"

#include "material.h"
#include "phongmaterial.h"
#include "pbrmaterial.h"

#include "mesh.h"
#include "model.h"

#include "textmesh.h"

#include "meshex.h"
#include "modelex.h"

#include "renderable.h"

// Renderer
#include "lightdata.h"
#include "skybox.h"
#include "coloredskybox.h"
#include "sixsidedskybox.h"

#include "shader.h"
#include "softwareshader.h"
#include "unlitshader.h"
#include "phongshader.h"

#include "rendertarget.h"
#include "rendercontext.h"
#include "renderer.h"

#include "softwarerenderer.h"
#include "raytracer.h"

// Assets
#include "factory.h"
#include "filereader.h"
#include "texturefactory.h"
#include "objloader.h"

// Win32
#include "gdibuffer.h"
#include "window.h"

// OpenGL
#include "mesh_gl.h"
#include "texture_gl.h"
#include "window_gl.h"
#include "shader_gl.h"
#include "program_gl.h"
#include "renderer_gl.h"

using namespace AwesomeRenderer;

int main() 
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Open window
	printf("[AwesomeRenderer]: Creating Win32 window...\n");
	
	Window window(hInstance, "AwesomeRendererWindow");
	window.Create("Awesome Renderer!", 100, 100);
	window.SetClientSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	printf("[AwesomeRenderer]: Creating OpenGL window...\n");

	WindowGL windowGL(window);
	
	if (!windowGL.Setup())
		printf("[AwesomeRenderer]: Failed to create OpenGL window!\n");

	InputManager& inputManager = InputManager::Instance();

	// Setup frame and depth buffers
	printf("[AwesomeRenderer]: Initializing frame buffer...\n");

	GdiBuffer frameBuffer(window.handle);
	MemoryBuffer depthBuffer;
	
	frameBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, Buffer::BGR24);
	depthBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, Buffer::FLOAT32);

	// Render target
	printf("[AwesomeRenderer]: Setting up render context...\n");

	RenderTarget renderTarget;
	renderTarget.SetupBuffers(&frameBuffer, &depthBuffer);

	// Setup camera
	const Vector3 cameraPosition = Vector3(3.2f, 1.8f, 7.5f);
	Camera camera(cml::left_handed);
	camera.SetLookAt(cameraPosition, cameraPosition - Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));
	camera.SetPerspective(45.0f, ((float) SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 5000.0f);
	camera.SetViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Render context
	RenderContext mainContext;
	mainContext.camera = &camera;
	mainContext.renderTarget = &renderTarget;

	/**/
	Camera cameraHud(cml::left_handed);
	cameraHud.viewMtx.identity();
	cml::matrix_set_translation(cameraHud.viewMtx, -SCREEN_WIDTH / 2.0f, -SCREEN_HEIGHT / 2.0f, 0.0f);

	cameraHud.SetOrthographic(SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 10.0f);
	cameraHud.SetViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);

	RenderContext hudContext;
	hudContext.camera = &cameraHud;
	hudContext.renderTarget = &renderTarget;
	hudContext.clearFlags = RenderTarget::BUFFER_DEPTH;
	/**/

	// Initialize renderers
	SoftwareRenderer softwareRenderer;
	RendererGL rendererGL;
	RayTracer rayTracer;
	
	const uint32_t NUM_RENDERERS = 3;
	const char* RENDERER_NAMES[] = { "Software renderer", "OpenGL renderer", "Raytracer" };

	Renderer* renderers[NUM_RENDERERS];
	renderers[0] = &softwareRenderer;
	renderers[1] = &rendererGL;
	renderers[2] = &rayTracer;

	for (uint32_t rendererIdx = 0; rendererIdx < NUM_RENDERERS; ++rendererIdx)
	{
		printf("[AwesomeRenderer]: Initializing %s...\n", RENDERER_NAMES[rendererIdx]);

		Renderer* renderer = renderers[rendererIdx];
		renderer->Initialize();
				
		renderer->cullMode = Renderer::CULL_NONE;
		//renderer->drawMode = Renderer::DRAW_LINE;
	}

	Renderer* mainRenderer = &rayTracer;

	// Shader
	PhongShader phongShader;
	UnlitShader unlitShader;

	// Lighting
	LightData lightData;
	lightData.numPixelLights = 8;
	lightData.ambient = Color::BLACK;// Color(0.1f, 0.1f, 0.1f);

	mainContext.lightData = &lightData;

	Vector3 zero(0.0f, 0.0f, 0.0f);

	{
		LightData::Light& light = lightData.lights[0];
		/*
		light.type = LightData::LightType::POINT;
		light.position = Vector3(5.0f, 3.0f, 5.0f);
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.1f;
		light.quadricAttenuation = 0.02f;
		light.intensity = 5.0f;
		/*/
		light.type = LightData::LightType::DIRECTIONAL;
		light.direction = Vector3(-0.5f, -0.8f, -0.5f);
		light.direction.normalize();
		light.intensity = 2.0f;
		//*/

		light.color = Color::WHITE;// Color(254, 253, 189);
		light.enabled = true;
	}

	{
		LightData::Light& light = lightData.lights[1];
		light.position = Vector3(5.0f, 3.0f, 5.0f);
		light.type = LightData::LightType::POINT;
		light.color = Color::WHITE;// Color::BLUE;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.1f;
		light.quadricAttenuation = 0.02f;
		light.intensity = 0.1f;
		light.enabled = false;
	}

	{
		LightData::Light& light = (lightData.lights[2] = lightData.lights[1]);
		light.position = Vector3(5.0f, 3.0f, -5.0f);
		light.color = Color::WHITE;//Color::RED;
	}

	{
		LightData::Light& light = (lightData.lights[3] = lightData.lights[1]);
		light.position = Vector3(-5.0f, 3.0f, -5.0f);
		light.color = Color::WHITE;//Color::PURPLE;
	}

	{
		LightData::Light& light = (lightData.lights[4] = lightData.lights[1]);
		light.position = Vector3(-5.0f, 3.0f, 5.0f);
		light.color = Color::WHITE;//Color::GREEN;
	}

	{
		LightData::Light& light = lightData.lights[5];
		light.position = Vector3(0.0f, 2.0f, 0.0f);
		light.type = LightData::LightType::POINT;
		light.color = Color(165, 250, 255);
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.1f;
		light.quadricAttenuation = 0.02f;
		light.intensity = 0.05f;
		light.enabled = false;
	}
		
	// Camera controller
	CameraController cameraController(camera);
	cameraController.CopyFromCamera();

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = &phongShader;
	
	// Game loop timer
	Timer timer(0.00001f, 100.0f);
	timer.Tick();

	// Skybox

	/*
	ColoredSkybox skybox;
	skybox.top = Color(35, 71, 189) * 0.5f;
	skybox.bottom = Color(107, 205, 209) * 0.5f;
	*/

	SixSidedSkybox skybox;
	skybox.right = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0001.bmp");
	skybox.front = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0002.bmp");
	skybox.top = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0003.bmp");
	skybox.left = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0004.bmp");
	skybox.back = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0005.bmp");
	skybox.bottom = textureFactory.GetTexture("../Assets/Skyboxes/sun25deg/skyrender0006.bmp");

	mainContext.skybox = &skybox;
	
	if (FALSE)
	{
		Node* node = new Node();
		Model* model = new Model();

		Transformation* transform = new Transformation();

		node->AddComponent(model);
		node->AddComponent(transform);

		//transform->SetScale(Vector3(0.1f, 0.1f, 0.1f));
		//transform->SetScale(Vector3(0.2f, 0.2f, 0.2f));
		//objLoader.Load("../Assets/Town/town.obj", *model);
		objLoader.Load("../Assets/crytek-sponza/sponza.obj", *model);
		//objLoader.Load("../Assets/Castle01/castle.obj", *model);

		mainContext.nodes.push_back(node);
	}
	
	if (FALSE)
	{
		Node* node = new Node();

		Texture* texture = NULL;
		textureFactory.GetAsset("../Assets/font.bmp", &texture);

		TextMesh* mesh = new TextMesh();
		mesh->Configure(texture, 32, 32, 1);
		mesh->SetText("Lorem ipsum dolor sit amet.");

		Sampler* sampler = new Sampler();
		sampler->sampleMode = Sampler::SM_POINT;
		sampler->wrapMode = Sampler::WM_REPEAT;
		sampler->texture = texture;

		PhongMaterial* material = new PhongMaterial();
		material->shader = &unlitShader;
		material->diffuseMap = sampler;

		Model* model = new Model();
		model->AddMesh(mesh, material);
		
		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(1.0f, 0.0f, 5.0f));
		transform->SetScale(Vector3(1.0f, 1.0f, 1.0f) * 0.4f);

		node->AddComponent(model);
		node->AddComponent(transform);

		hudContext.nodes.push_back(node);
	}
	
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
		textureFactory.GetAsset("../Assets/tiles.bmp", &texture);

		Sampler* sampler = new Sampler();
		sampler->texture = texture;

		PhongMaterial* material = new PhongMaterial();
		material->shader = &phongShader;
		material->diffuseMap = sampler;
		material->specularColor = Color::WHITE * 0.1f;
		material->shininess = 1.0f;

		Model* model = new Model();
		model->AddMesh(mesh, material);
		model->CalculateBounds();
		
		node->AddComponent(model);

		Transformation* transform = new Transformation();
		transform->SetScale(Vector3(5.0f, 5.0f, 5.0f));
		node->AddComponent(transform);

		mainContext.nodes.push_back(node);
	}

	{
		Node* node = new Node();

		Transformation* transform = new Transformation();
		node->AddComponent(transform);

		PbrMaterial* floorMaterial = new PbrMaterial();
		floorMaterial->albedo = Color::BLACK;
		floorMaterial->specular = Color::WHITE * 0.8f;
		floorMaterial->metallic = 1.0f;
		floorMaterial->roughness = 0.7f;

		Renderable* renderable = new Renderable();
		renderable->primitive = new Plane(0.0f, Vector3(0.0f, 1.0f, 0.0f));
		
		renderable->material = floorMaterial;

		node->AddComponent(renderable);

		//mainContext.nodes.push_back(node);
	}

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
			position[0] += sphereIdx * (SPHERE_RADIUS * 2 + SPHERE_SPACING);
			position[1] += row * (SPHERE_RADIUS * 2 + SPHERE_SPACING);

			Transformation* transform = new Transformation();
			transform->SetPosition(position);
			node->AddComponent(transform);

			PbrMaterial* material = new PbrMaterial();
			material->roughness = MIN_ROUGHNESS + (sphereIdx / (float)(SPHERES_PER_ROW - 1.0f)) * (1.0f - MIN_ROUGHNESS);

			if (row == 0)
			{
				material->albedo = Color::WHITE * 0.8f;
				material->specular = Color::WHITE * 0.2f;
				material->metallic = 0;
			}
			else if (row == 1)
			{
				material->albedo = Color::BLACK;
				material->specular = Color::WHITE * 0.9f;
				material->metallic = 1;
			}

			Renderable* renderable = new Renderable();
			renderable->primitive = new Sphere(Vector3(0.0f, 0.0f, 0.0f), SPHERE_RADIUS);

			renderable->material = material;

			node->AddComponent(renderable);

			mainContext.nodes.push_back(node);
		}
	}

	// Convert all meshes to OpenGL meshes
	std::vector<RenderContext*> contexts = { &mainContext };
	
	for (uint32_t contextIdx = 0; contextIdx < contexts.size(); ++contextIdx)
	{
		RenderContext& renderContext = *contexts[contextIdx];

		for (auto nodeIt = renderContext.nodes.begin(); nodeIt != renderContext.nodes.end(); ++nodeIt)
		{
			Model* model = (*nodeIt)->GetComponent<Model>();

			if (model == NULL)
				continue;

			for (auto meshIt = model->meshes.begin(); meshIt != model->meshes.end(); ++meshIt)
			{
				MeshGL* meshGL = new MeshGL(**meshIt);
				meshGL->CreateBuffers();
			}

			for (auto materialIt = model->materials.begin(); materialIt != model->materials.end(); ++materialIt)
			{
				// TODO: create texture map in base Material class so that this can work on the base class
				PhongMaterial* material = static_cast<PhongMaterial*>(*materialIt);

				if (material == NULL)
					continue;

				TextureGL* texture;

				if (material->diffuseMap != NULL && !material->diffuseMap->texture->HasExtension())
				{
					texture = new TextureGL(*material->diffuseMap->texture);
					texture->Load();
				}

				if (material->normalMap != NULL && !material->normalMap->texture->HasExtension())
				{
					texture = new TextureGL(*material->normalMap->texture);
					texture->Load();
				}

				if (material->specularMap != NULL && !material->specularMap->texture->HasExtension())
				{
					texture = new TextureGL(*material->specularMap->texture);
					texture->Load();
				}
			}
		}
	}

	window.Show(nCmdShow);

	printf("[AwesomeRenderer]: Initialization done!\n");

	// Frame counter variables
	float timeSinceLastPrint = 0.0f;
	uint32_t framesDrawn = 0;
	
	while (!window.closed)
	{
		const TimingInfo& timingInfo = timer.Tick();

		// Frame counter
		++framesDrawn;
		timeSinceLastPrint += timingInfo.elapsedSeconds;

		if (timeSinceLastPrint >= 0.5f)
		{
			if (mainRenderer == &rayTracer)
				printf("[RayTracer]: Frame progress: %.0f%%\n", rayTracer.GetProgress() * 100);
			else
				printf("[AwesomeRenderer]: FPS: %d; Last frame time: %dms;\n", framesDrawn, (uint32_t)(timingInfo.elapsedSeconds * 1000.0f));

			framesDrawn = 0;
			timeSinceLastPrint -= 1.0f;
		}

		// Updating logic
		cameraController.Update(timingInfo);
		camera.UpdateViewMtx();

		// Keyboard light switching
		for (uint32_t lightIdx = 0; lightIdx < LightData::MAX_LIGHTS; ++lightIdx)
		{
			if (inputManager.GetKeyDown(VK_NUMPAD0 + lightIdx))
			{
				LightData::Light& light = mainContext.lightData->lights[lightIdx];
				light.enabled = !light.enabled;
			}
		}

		// Keyboard renderer switching
		if (inputManager.GetKey('I'))
			mainRenderer = renderers[0];

		if (inputManager.GetKey('O'))
			mainRenderer = renderers[1];

		if (inputManager.GetKey('P'))
			mainRenderer = renderers[2];

		bool plus = inputManager.GetKeyDown(VK_OEM_PLUS);
		bool minus = inputManager.GetKeyDown(VK_OEM_MINUS);
		bool shift = inputManager.GetKey(VK_SHIFT);

		if (plus || minus)
		{
			if (shift)
			{
				if (plus)
					rayTracer.sampleCount <<= 1;
				else if (rayTracer.sampleCount > 1)
					rayTracer.sampleCount >>= 1;

				printf("[AwesomeRenderer]: Settings raytracer sample count to %d\n", rayTracer.sampleCount);
			}
			else
			{
				if (plus)
					++rayTracer.maxDepth;
				else if (rayTracer.maxDepth > 0)
					--rayTracer.maxDepth;

				printf("[AwesomeRenderer]: Settings raytracer max depth to %d\n", rayTracer.maxDepth);
			}
		}


		for (uint32_t contextIdx = 0; contextIdx < contexts.size(); ++contextIdx)
		{
			RenderContext& renderContext = *contexts[contextIdx];

			// Prepare models in scene
			std::vector<Node*>::iterator it;

			for (it = renderContext.nodes.begin(); it != renderContext.nodes.end(); ++it)
			{
				Node& node = **it;
				Transformation* transform = node.GetComponent<Transformation>();

				if (transform == NULL)
					continue;

				// Update global transformation matrix
				transform->CalculateMtx();

				// Update model
				Model* model = node.GetComponent<Model>();

				if (model != NULL)
				{
					// Iterate through submeshes in a node
					for (uint32_t cMesh = 0; cMesh < model->meshes.size(); ++cMesh)
					{
						// Transform bounding shape of mesh according to world transformation
						Mesh* mesh = model->meshes[cMesh];
						mesh->bounds.Transform(transform->WorldMtx());
					}

					model->bounds.Transform(transform->WorldMtx());
				}

				// Update renderable object
				Renderable* renderable = node.GetComponent<Renderable>();

				if (renderable != NULL)
				{
					if (renderable->primitive != NULL)
						renderable->primitive->Transform(transform->WorldMtx());
				}
			}

			mainRenderer->SetRenderContext(&renderContext);
			mainRenderer->Render();
		}

		mainRenderer->Present(window);

		// Present window
		window.ProcessMessages();
	}

	for (uint32_t rendererIdx = 0; rendererIdx < NUM_RENDERERS; ++rendererIdx)
	{
		Renderer* renderer = renderers[rendererIdx];
		renderer->Cleanup();
	}

	frameBuffer.Destroy();
	depthBuffer.Destroy();

	return 0;
}
