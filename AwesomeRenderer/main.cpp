#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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
#include "shape.h"
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

#include "setup.h"

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
	Camera camera(cml::left_handed);
	camera.SetPerspective(70.0f, ((float) SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 5000.0f);
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
	
	// Lighting
	LightData lightData;
	SetupLighting(lightData);
	mainContext.lightData = &lightData;

	Vector3 zero(0.0f, 0.0f, 0.0f);

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = new PhongShader();
	
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
	
	SetupScene(mainContext, hudContext, objLoader, textureFactory);
	SetupCornellBox(mainContext, camera);
	//SetupSpheres(mainContext, camera);

	/**/
	UnlitShader unlitShader;
	TextMesh* debugText;

	{
		Node* node = new Node();

		Texture* texture = NULL;
		textureFactory.GetAsset("../Assets/font.bmp", &texture);

		debugText = new TextMesh();
		debugText->Configure(texture, 32, 32, 1);
		debugText->SetText("Initializing...");

		Sampler* sampler = new Sampler();
		sampler->sampleMode = Sampler::SM_POINT;
		sampler->wrapMode = Sampler::WM_REPEAT;
		sampler->texture = texture;

		PhongMaterial* material = new PhongMaterial(*(new Material()));
		material->provider.shader = &unlitShader;
		material->diffuseMap = sampler;

		Model* model = new Model();
		model->AddMesh(debugText, &material->provider);

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(1.0f, 0.0f, 5.0f));
		transform->SetScale(Vector3(1.0f, 1.0f, 1.0f) * 0.4f);

		node->AddComponent(model);
		node->AddComponent(transform);

		hudContext.nodes.push_back(node);
	}
	/**/

	// Camera controller
	CameraController cameraController(camera);
	cameraController.CopyFromCamera();

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
				PhongMaterial* material = (*materialIt)->As<PhongMaterial>();

				if (material == NULL)
					continue;

				TextureGL* texture;

				if (material->diffuseMap != NULL)
				{
					texture = new TextureGL(*material->diffuseMap->texture);
					texture->Load();
				}

				if (material->normalMap != NULL)
				{
					texture = new TextureGL(*material->normalMap->texture);
					texture->Load();
				}

				if (material->specularMap != NULL)
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
	char textBuffer[512];
	
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

			rayTracer.ResetFrame();

			sprintf(textBuffer, "Bounces: %u; SPP: %u", rayTracer.maxDepth, rayTracer.sampleCount);
			debugText->SetText(textBuffer);
		}


		mainContext.Update();
		mainRenderer->SetRenderContext(&mainContext);
		mainRenderer->Render();
		
		hudContext.Update();
		softwareRenderer.SetRenderContext(&hudContext);
		softwareRenderer.Render();

		window.DrawBuffer(frameBuffer);
		//windowGL.Draw();

		// Process Win32 messages
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
