#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "awesomerenderer.h"

using namespace AwesomeRenderer;

void SetupConsole()
{
	AllocConsole();

	// Magic!
	// (Opens a console window)
    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetupConsole();

	SoftwareRenderer softwareRenderer;
	RayTracer rayTracer;

	// Open window
	Window window(hInstance, "AwesomeRendererWindow");
	window.Create("Awesome Renderer!", SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Setup frame and depth buffers
	GdiBuffer frameBuffer(window.handle);
	MemoryBuffer depthBuffer;
	
	frameBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, 3);	// 24 bit RGB buffer
	depthBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, 2);	// 16 bit depth buffer

	// Render target
	RenderTarget renderTarget;
	renderTarget.SetupBuffers(&frameBuffer, &depthBuffer);

	// Setup camera
	Camera camera(cml::left_handed);
	camera.SetLookAt(Vector3(0.0f, 5.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0));
	camera.SetPerspective(45.0f, ((float) SCREEN_WIDTH) / SCREEN_HEIGHT, 1.0f, 500.0f);
	camera.SetViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Render context
	RenderContext renderContext;
	renderContext.camera = &camera;
	renderContext.renderTarget = &renderTarget;

	softwareRenderer.renderContext = &renderContext;
	rayTracer.renderContext = &renderContext;
	
	// Camera controller
	CameraController cameraController(camera);
	cameraController.distance = 20.0f;

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	
	// Game loop timer
	Timer timer;

	// Test model
	Node car;
	objLoader.Load("../Assets/car.obj", car.model);

	renderContext.nodes.push_back(&car);
	
	window.Show(nCmdShow);

	float timeSinceLastPrint = 0.0f;
	int framesDrawn = 0;
	while (!window.closed)
	{
		const TimingInfo& timingInfo = timer.Tick();

		// Updating logic
		cameraController.Update(timingInfo);
		camera.UpdateViewMtx();

		// Rendering
		renderTarget.Clear(Color::BLACK);

		//softwareRenderer.Render();
		rayTracer.Render();

		// Present window
		window.ProcessMessages();
		window.DrawBuffer(frameBuffer);
		
		// Frame counter
		++framesDrawn;
		timeSinceLastPrint += timingInfo.elapsedSeconds;

		if (timeSinceLastPrint >= 1.0f)
		{
			printf("FPS: %d\n", framesDrawn);
			framesDrawn = 0;
			timeSinceLastPrint -= 1.0f;
		}

	}

	frameBuffer.Destroy();
	depthBuffer.Destroy();

	return 0;
}
