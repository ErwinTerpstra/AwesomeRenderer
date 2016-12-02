#include "stdafx.h"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "awesomerenderer.h"

#include "buffer.h"
#include "memorybufferallocator.h"
#include "gdibufferallocator.h"

#include "scheduler.h"

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
#include "raytracerdebug.h"

// Assets
#include "factory.h"
#include "filereader.h"
#include "texturefactory.h"
#include "objloader.h"

// Win32
#include "gdibufferallocator.h"
#include "window.h"

// OpenGL
#include "mesh_gl.h"
#include "texture_gl.h"
#include "window_gl.h"
#include "shader_gl.h"
#include "program_gl.h"
#include "renderer_gl.h"
#include "rendertarget_gl.h"

#include "context.h"
#include "setup.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

int main() 
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	srand(time(0));

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

	// Job scheduler
	printf("[AwesomeRenderer]: Setting up scheduler...\n");
	Scheduler scheduler(0);
	scheduler.Start();

	// Setup frame and depth buffers
	printf("[AwesomeRenderer]: Initializing frame buffer...\n");

	Texture frameBuffer(new GDIBufferAllocator(window.handle));
	Texture depthBuffer(new MemoryBufferAllocator());
	
	frameBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, Buffer::BGR24);
	depthBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, Buffer::FLOAT32);

	// Render target
	printf("[AwesomeRenderer]: Setting up render context...\n");

	RenderTarget renderTarget;
	renderTarget.SetupBuffers(&frameBuffer, &depthBuffer);

	RenderTargetGL renderTargetGL(renderTarget);
	renderTargetGL.Load();

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
	RayTracer rayTracer(scheduler);
	
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

	Renderer* mainRenderer = &rendererGL;
	
	// Lighting
	LightData lightData;
	mainContext.lightData = &lightData;

	Vector3 zero(0.0f, 0.0f, 0.0f);

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = new PhongShader();
	
	// Game loop timer
	Timer timer(0.00001f, 100.0f);
	timer.Tick();
	
	Context context;
	context.mainCamera = &camera;
	context.mainContext = &mainContext;

	context.hudCamera = &cameraHud;
	context.hudContext = &hudContext;

	context.objLoader = &objLoader;
	context.textureFactory = &textureFactory;

	Setup setup(context);
	setup.SetupLighting();
	setup.SetupScene();
	//setup.SetupCornellBox();
	//setup.SetupSpheres();
	//setup.SetupFractal();
	setup.SetupSponza();
	
	// Camera controller
	CameraController cameraController(camera);
	cameraController.CopyFromCamera();

	// Debug
	RayTracerDebug rayTracerDebug(context, rayTracer);
	rayTracerDebug.Setup();

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

	mainContext.Optimize();

	window.Show(nCmdShow);

	printf("[AwesomeRenderer]: Initialization done!\n");

	// Frame counter variables
	float timeSinceLastPrint = 0.0f;
	uint32_t framesDrawn = 0;

	const char switchKeys[] = { 'I', 'O', 'P' };
		
	while (!window.closed)
	{
		const TimingInfo& timingInfo = timer.Tick();

		// Frame counter
		++framesDrawn;
		timeSinceLastPrint += timingInfo.elapsedSeconds;

		if (timeSinceLastPrint >= 1.0f)
		{
			if (mainRenderer != &rayTracer)
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
		for (uint32_t rendererIdx = 0; rendererIdx < NUM_RENDERERS; ++rendererIdx)
		{
			if (mainRenderer == renderers[rendererIdx])
				continue;

			if (inputManager.GetKey(switchKeys[rendererIdx]))
			{
				if (mainRenderer == &rayTracer)
					rayTracer.ResetFrame(false);

				mainRenderer = renderers[rendererIdx];

				printf("[AwesomeRenderer]: Rendering using %s\n", RENDERER_NAMES[rendererIdx]);
			}
		}

		rayTracerDebug.Update(timingInfo.elapsedSeconds);

		mainContext.Update();
		mainRenderer->SetRenderContext(&mainContext);
		mainRenderer->Render();
		
		//hudContext.Update();
		//softwareRenderer.SetRenderContext(&hudContext);
		//softwareRenderer.Render();

		window.DrawBuffer(frameBuffer, static_cast<const GDIBufferAllocator&>(frameBuffer.GetAllocator()));

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

	scheduler.Stop();

	return 0;
}
