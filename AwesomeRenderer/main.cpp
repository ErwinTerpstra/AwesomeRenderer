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

	// Shader
	PhongShader phongShader;
	phongShader.lightData.numPixelLights = 8;
	phongShader.lightData.ambient = Color(0.1f, 0.1f, 0.1f);

	{
		PhongShader::Light& light = phongShader.lightData.lights[0];
		light.position = Vector3(0.0f, 8.0f, 0.0f);
		light.type = PhongShader::LightType::POINT;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.01f;
		light.quadricAttenuation = 0.01f;
		light.intensity = 1.0f;
		light.enabled = true;
	}
	
	
	// Camera controller
	CameraController cameraController(camera);
	cameraController.distance = 20.0f;

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = &phongShader;
	
	// Game loop timer
	Timer timer;

	// Test models
	Node car;
	objLoader.Load("../Assets/car.obj", car.model);

	Node plane;
	{
		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL));
		mesh->AddQuad(Vector3(1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, -1.0f));

		Material* material = new Material();
		material->shader = &phongShader;

		plane.model.AddMesh(mesh, material);
		plane.transform.SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}

	//softwareRenderer.cullMode = Renderer::CULL_NONE;

	renderContext.nodes.push_back(&car);
	renderContext.nodes.push_back(&plane);
	
	window.Show(nCmdShow);

	// Frame counter variables
	float timeSinceLastPrint = 0.0f;
	int framesDrawn = 0;

	while (!window.closed)
	{
		const TimingInfo& timingInfo = timer.Tick();

		// Updating logic
		cameraController.Update(timingInfo);
		camera.UpdateViewMtx();

		// Prepare models in scene
		std::vector<Node*>::iterator it;

		for (it = renderContext.nodes.begin(); it != renderContext.nodes.end(); ++it)
		{
			Node& node = **it;
			node.transform.CalculateMtx();

			// Iterate through submeshes in a node
			for (unsigned int cMesh = 0; cMesh < node.model.meshes.size(); ++cMesh)
			{
				// Transform bounding shape of mesh according to world transformation
				Mesh& mesh = *node.model.meshes[cMesh];
				mesh.bounds.Transform(node.transform.WorldMtx());
			}
		}
		
		// Rendering
		renderTarget.Clear(Color::BLACK);

		softwareRenderer.Render();
		//rayTracer.Render();
		
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
