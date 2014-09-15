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

	// Open window
	Window window(hInstance, "AwesomeRendererWindow");
	window.Create("Awesome Renderer!", SCREEN_WIDTH, SCREEN_HEIGHT);
	
	//GLWindow glWindow(window);
	//glWindow.Setup();

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

	// Initialize renderer
	SoftwareRenderer softwareRenderer;
	softwareRenderer.Initialize();

	//RayTracer rayTracer;
	//GLRenderer glRenderer(glWindow);
	
	Renderer& renderer = softwareRenderer;
	renderer.SetRenderContext(&renderContext);

	// Shader
	PhongShader phongShader;
	phongShader.lightData.numPixelLights = 8;
	phongShader.lightData.ambient = Color(0.1f, 0.1f, 0.1f);

	Vector3 zero(0.0f, 0.0f, 0.0f);

	{

		PhongShader::Light& light = phongShader.lightData.lights[0];
		light.position = Vector3(5.0f, 10.0f, 0.0f);
		light.direction = (zero - light.position).normalize();
		light.type = PhongShader::LightType::POINT;
		light.angle = (float) (40.0f * (M_PI / 180.0f));
		light.angleExponent = 20.0f;
		light.color = Color::WHITE;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.05f;
		light.quadricAttenuation = 0.04f;
		light.intensity = 4.0f;
		light.enabled = true;
	}

	{
		PhongShader::Light& light = phongShader.lightData.lights[1];
		light.position = Vector3(5.0f, 3.0f, 5.0f);
		light.direction = (zero - light.position).normalize();
		light.type = PhongShader::LightType::POINT;
		light.angle = (float) (30.0f * (M_PI / 180.0f));
		light.angleExponent = 20.0f;
		light.color = Color::BLUE;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.0f;
		light.quadricAttenuation = 2.0f;
		light.intensity = 8.0f;
		light.enabled = false;
	}

	{

		PhongShader::Light& light = phongShader.lightData.lights[2];
		light.position = Vector3(-5.0f, 3.0f, -5.0f);
		light.direction = (zero - light.position).normalize();
		light.type = PhongShader::LightType::POINT;
		light.angle = (float) (30.0f * (M_PI / 180.0f));
		light.angleExponent = 20.0f;
		light.color = Color::RED;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.0f;
		light.quadricAttenuation = 2.0f;
		light.intensity = 8.0f;
		light.enabled = true;
	}
	
	
	// Camera controller
	CameraController cameraController(camera);

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = &phongShader;
	
	// Game loop timer
	Timer timer(0.00001f, 100.0f);
	timer.Tick();

	// Test models
	Node car;
	car.model = new Model();
	car.transform = new Transformation();
	objLoader.Load("../Assets/car.obj", *car.model);

	/*
	ModelEx modelEx(*car.model);

	for (std::vector<MeshEx*>::iterator meshIterator = modelEx.meshes.begin(); meshIterator != modelEx.meshes.end(); ++meshIterator)
		(*meshIterator)->OptimizeTree();
	*/


	Node plane;
	{
		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL | Mesh::VERTEX_TEXCOORD));
		mesh->AddQuad(Vector3(1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, -1.0f));
		
		float uvScale = 5.0f;
		mesh->texcoords[0] = Vector2(1.0f, 1.0f) * uvScale;
		mesh->texcoords[1] = Vector2(0.0f, 1.0f) * uvScale;
		mesh->texcoords[2] = Vector2(0.0f, 0.0f) * uvScale;

		mesh->texcoords[3] = Vector2(0.0f, 0.0f) * uvScale;
		mesh->texcoords[4] = Vector2(1.0f, 0.0f) * uvScale;
		mesh->texcoords[5] = Vector2(1.0f, 1.0f) * uvScale;

		Texture* texture = new Texture();
		textureFactory.Load("../Assets/tiles.bmp", *texture);

		Sampler* sampler = new Sampler();
		sampler->texture = texture;

		Material* material = new Material();
		material->shader = &phongShader;
		material->diffuseMap = sampler;
		material->specularColor = Color::WHITE;
		material->shininess = 50.0f;

		plane.model = new Model();
		plane.model->AddMesh(mesh, material);

		plane.transform = new Transformation();
		plane.transform->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}

	renderer.cullMode = Renderer::CULL_NONE;

	renderContext.nodes.push_back(&car);
	renderContext.nodes.push_back(&plane);
	
	window.Show(nCmdShow);

	// Frame counter variables
	float timeSinceLastPrint = 0.0f;
	uint32_t framesDrawn = 0;

	while (!window.closed)
	{
		const TimingInfo& timingInfo = timer.Tick();

		// Frame counter
		++framesDrawn;
		timeSinceLastPrint += timingInfo.elapsedSeconds;

		if (timeSinceLastPrint >= 1.0f)
		{
			printf("FPS: %d; Last frame time: %dms;\n", framesDrawn, (uint32_t)(timingInfo.elapsedSeconds * 1000.0f));
			framesDrawn = 0;
			timeSinceLastPrint -= 1.0f;
		}

		// Updating logic
		cameraController.Update(timingInfo);
		camera.UpdateViewMtx();

		// Prepare models in scene
		std::vector<Node*>::iterator it;

		for (it = renderContext.nodes.begin(); it != renderContext.nodes.end(); ++it)
		{
			Node& node = **it;
			node.transform->CalculateMtx();

			// Iterate through submeshes in a node
			for (unsigned int cMesh = 0; cMesh < node.model->meshes.size(); ++cMesh)
			{
				// Transform bounding shape of mesh according to world transformation
				Mesh& mesh = *node.model->meshes[cMesh];
				mesh.bounds.Transform(node.transform->WorldMtx());
			}
		}
		
		// Rendering
		renderTarget.Clear(Color::BLACK);

		renderer.Render();
		
		// Present window
		window.ProcessMessages();
		window.DrawBuffer(frameBuffer);
	}

	softwareRenderer.Cleanup();

	frameBuffer.Destroy();
	depthBuffer.Destroy();

	return 0;
}
