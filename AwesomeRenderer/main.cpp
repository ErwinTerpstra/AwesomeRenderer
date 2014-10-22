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
	
	WindowGL windowGL(window);
	windowGL.Setup();

	// Setup frame and depth buffers
	GdiBuffer frameBuffer(window.handle);
	MemoryBuffer depthBuffer;
	
	frameBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, 3);	// 24 bit RGB buffer
	depthBuffer.Allocate(SCREEN_WIDTH, SCREEN_HEIGHT, 4);	// 32 bit depth buffer

	// Render target
	RenderTarget renderTarget;
	renderTarget.SetupBuffers(&frameBuffer, &depthBuffer);

	// Setup camera
	Camera camera(cml::left_handed);
	camera.SetLookAt(Vector3(-5.0f, 5.0f, 5.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0));
	camera.SetPerspective(45.0f, ((float) SCREEN_WIDTH) / SCREEN_HEIGHT, 0.1f, 5000.0f);
	camera.SetViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Render context
	RenderContext renderContext;
	renderContext.camera = &camera;
	renderContext.renderTarget = &renderTarget;
	renderContext.window = &window;

	// Initialize renderer
	SoftwareRenderer softwareRenderer;
	RendererGL rendererGL;

	Renderer* renderer = &rendererGL;

	const uint32_t NUM_RENDERERS = 2;
	Renderer* renderers[NUM_RENDERERS];
	renderers[0] = &softwareRenderer;
	renderers[1] = &rendererGL;

	for (uint32_t rendererIdx = 0; rendererIdx < NUM_RENDERERS; ++rendererIdx)
	{
		Renderer* renderer = renderers[rendererIdx];
		renderer->Initialize();

		renderer->SetRenderContext(&renderContext);
		
		renderer->cullMode = Renderer::CULL_NONE;
		//renderer->drawMode = Renderer::DRAW_LINE;
	}

	Renderer* activeRenderer = &rendererGL;

	// Shader
	PhongShader phongShader;
	phongShader.lightData.numPixelLights = 8;
	phongShader.lightData.ambient = Color(0.1f, 0.1f, 0.1f);

	UnlitShader unlitShader;

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
		light.direction = Vector3(0.0f, -0.5f, -0.5f);
		light.type = PhongShader::LightType::DIRECTIONAL;
		light.color = Color::WHITE;
		light.intensity = 4.0f;
		light.enabled = true;

		light.direction.normalize();
	}

	// Camera controller
	CameraController cameraController(camera);
	cameraController.CopyFromCamera();

	// Assets factories
	TextureFactory textureFactory;
	ObjLoader objLoader(textureFactory);
	objLoader.defaultShader = &unlitShader;
	
	// Game loop timer
	Timer timer(0.00001f, 100.0f);
	timer.Tick();


	/*
	{
		Node node;
		node.model = new Model();
		node.transform = new Transformation();
		objLoader.Load("../Assets/Town/town.obj", *node.model);

		renderContext.nodes.push_back(&node);
	}
	//*/

	/*
	{
		Node node;
		node.model = new Model();
		node.transform = new Transformation();
		node.transform->SetScale(Vector3(0.1f, 0.1f, 0.1f));
		objLoader.Load("../Assets/Castle01/castle.obj", *node.model);

		renderContext.nodes.push_back(&node);
	}
	//*/

	//*
	{
		Node node;
		node.model = new Model();
		node.transform = new Transformation();
		node.transform->SetScale(Vector3(0.2f, 0.2f, 0.2f));
		objLoader.Load("../Assets/crytek-sponza/sponza.obj", *node.model);

		renderContext.nodes.push_back(&node);
	}
	//*/

	/*
	{
		Node node;

		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL | Mesh::VERTEX_TEXCOORD));
		mesh->AddQuad(Vector3(1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, -1.0f));

		float uvScale = 5.0f;
		mesh->texcoords[0] = Vector2(1.0f, 1.0f) * uvScale;
		mesh->texcoords[1] = Vector2(0.0f, 1.0f) * uvScale;
		mesh->texcoords[2] = Vector2(0.0f, 0.0f) * uvScale;

		mesh->texcoords[3] = Vector2(0.0f, 0.0f) * uvScale;
		mesh->texcoords[4] = Vector2(1.0f, 0.0f) * uvScale;
		mesh->texcoords[5] = Vector2(1.0f, 1.0f) * uvScale;

		Texture* texture = NULL;
		textureFactory.GetAsset("../Assets/tiles.bmp", &texture);

		Sampler* sampler = new Sampler();
		sampler->texture = texture;

		Material* material = new Material();
		material->shader = &unlitShader;
		material->diffuseMap = sampler;
		material->specularColor = Color::WHITE;
		material->shininess = 50.0f;

		node.model = new Model();
		node.model->AddMesh(mesh, material);

		node.transform = new Transformation();
		node.transform->SetScale(Vector3(10.0f, 10.0f, 10.0f));

		renderContext.nodes.push_back(&node);
	}
	//*/

	// Convert all meshes to OpenGL meshes
	for (auto nodeIt = renderContext.nodes.begin(); nodeIt != renderContext.nodes.end(); ++nodeIt)
	{
		Model* model = (*nodeIt)->model;
		
		for (auto meshIt = model->meshes.begin(); meshIt != model->meshes.end(); ++meshIt)
		{
			MeshGL* meshGL = new MeshGL(**meshIt);
			meshGL->CreateBuffers();
		}

		for (auto materialIt = model->materials.begin(); materialIt != model->materials.end(); ++materialIt)
		{
			Material* material = (*materialIt);
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
		
		// Renderer switch keyhandling
		for (uint32_t rendererIdx = 0; rendererIdx < NUM_RENDERERS; ++rendererIdx)
		{
			if (InputManager::Instance().GetKey(VK_NUMPAD0 + rendererIdx))
				activeRenderer = renderers[rendererIdx];
		}

		activeRenderer->Render();
		
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
