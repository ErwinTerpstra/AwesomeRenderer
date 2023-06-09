

#include "raytracerdebug.h"

#include "inputmanager.h"
#include "raytracer.h"

#include "node.h"
#include "texturefactory.h"
#include "sampler.h"
#include "memorybufferallocator.h"
#include "transformation.h"
#include "textmesh.h"
#include "phongmaterial.h"
#include "model.h"
#include "context.h"
#include "surfaceintegrator.h"

#include "sphere.h"

#include "random.h"
#include "lambert.h"
#include "microfacetspecular.h"
#include "microfacetmaterial.h"

#include <WinUser.h>

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

const std::string RayTracerDebug::RENDER_ROOT = "../Renders";
const float RayTracerDebug::UPDATE_INTERVAL = 0.2f;


RayTracerDebug::RayTracerDebug(Context& context, RayTracer& rayTracer) : 
	context(context), rayTracer(rayTracer), inputManager(InputManager::Instance()),
	currentIntegrator(0), textBuffer(NULL), exportMode(DISABLED)
{
	integrators[0] = &rayTracer.debugIntegrator;
	integrators[1] = &rayTracer.whittedIntegrator;
	integrators[2] = &rayTracer.monteCarloIntegrator;
}

RayTracerDebug::~RayTracerDebug()
{
	if (textBuffer != NULL)
	{
		delete textBuffer;
		textBuffer = NULL;
	}
}

void RayTracerDebug::Setup()
{
	textBuffer = new char[TEXT_BUFFER_SIZE];

	SetupDebugDisplay();
}

void RayTracerDebug::Update(float dt)
{
	const RenderContext* renderContext = &rayTracer.GetRenderContext();

	if (renderContext == NULL)
		return;

	timeSinceUpdate += dt;

	if (inputManager.GetKeyDown('G'))
	{
		currentIntegrator = (currentIntegrator + 1) % INTEGRATOR_COUNT;
		rayTracer.currentIntegrator = integrators[currentIntegrator];

		rayTracer.ResetFrame();
	}
	
	bool plus = inputManager.GetKeyDown(VK_OEM_PLUS);
	bool minus = inputManager.GetKeyDown(VK_OEM_MINUS);
	bool shift = inputManager.GetKey(VK_SHIFT);

	if (plus || minus)
	{
		if (shift)
		{
			if (plus)
				rayTracer.samplesPerPixel <<= 1;
			else if (rayTracer.samplesPerPixel > 1)
				rayTracer.samplesPerPixel >>= 1;

			printf("[AwesomeRenderer]: Settings raytracer sample count to %d\n", rayTracer.samplesPerPixel);
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

		UpdateDebugDisplay();
	}

	if (inputManager.GetKeyDown('L'))
	{
		exportMode = (ExportMode) ((exportMode + 1) % EXPORT_MODE_COUNT);
		UpdateDebugDisplay();
	}

	if (inputManager.GetKey(VK_CONTROL) && inputManager.GetKeyDown(InputManager::LEFT_MOUSE_BUTTON))
	{
		const Buffer& frameBuffer = *context.mainContext->renderTarget->frameBuffer;

		Point2 debugPixel = inputManager.GetMousePosition();
		debugPixel[1] = frameBuffer.height - debugPixel[1];

		rayTracer.debugPixel = debugPixel;
		rayTracer.Render(debugPixel);
	}

	if (!rayTracer.IsRenderingFrame())
	{
		if (exportMode != DISABLED)
		{
			Export();

			if (exportMode == ONCE)
				exportMode = DISABLED;

			UpdateDebugDisplay();
		}
	}

	if (timeSinceUpdate >= UPDATE_INTERVAL)
	{
		UpdateDebugDisplay();
		timeSinceUpdate = 0.0f;
	}
}

void RayTracerDebug::SetupDebugDisplay()
{
	Node* node = new Node();

	Texture* texture = NULL;
	context.textureFactory->GetAsset("../Assets/font.bmp", &texture);

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
	transform->SetScale(Vector3(1.0f, 1.0f, 1.0f) * 0.5f);

	node->AddComponent(model);
	node->AddComponent(transform);

	context.hudContext->nodes.push_back(node);

	UpdateDebugDisplay();
}

void RayTracerDebug::UpdateDebugDisplay()
{
	float progress = rayTracer.GetProgress();
	std::string timeLeft;
	
	if (progress > 1e-3f)
		timeLeft = FormatTime(rayTracer.FrameTime() * ((1.0f - progress) / progress));
	else
		timeLeft = "N/A";

	sprintf(textBuffer,
		"Bounces: %u; SPP: %u;\nEst. time left: %s\nProgress: %.0f%%\nExport: %s",
		rayTracer.maxDepth, rayTracer.samplesPerPixel,
		timeLeft.c_str(),
		progress * 100,
		exportMode == CONTINUOUS ? "continuous" : (exportMode == ONCE ? "once" : "disabled")
	);

	debugText->SetText(textBuffer);
}

void RayTracerDebug::Export()
{
	time_t t = time(0);
	tm* now = localtime(&t);

	sprintf(textBuffer, "render %d-%02d-%02d %02d-%02d-%02d", 1900 + now->tm_year, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	std::string identifier(textBuffer);

	// Save image
	const Buffer& frameBuffer = *context.mainContext->renderTarget->frameBuffer;
	
	Buffer alignedBuffer(new MemoryBufferAllocator(), Buffer::GAMMA);
	alignedBuffer.AllocateAligned(frameBuffer.width, frameBuffer.height, 4, Buffer::BGR24);
	alignedBuffer.Blit(frameBuffer);

	std::string imageFileName = RENDER_ROOT + "/" + identifier + ".bmp";
	context.textureFactory->WriteBMP(imageFileName, alignedBuffer);
	
	alignedBuffer.Destroy();

	// Write stats
	std::string statsFileName = RENDER_ROOT + "/" + identifier + ".txt";
		
	FILE* filePtr;

	// Open filename in read binary mode 
	errno_t result = fopen_s(&filePtr, statsFileName.c_str(), "w");

	if (result != 0)
	{
		printf("[RayTracerDebug]: Failed to open file \"%s\". Error code: %d\n", statsFileName.c_str(), result);
		return;
	}

	std::string frameTime = FormatTime(rayTracer.FrameTime());

	fprintf(filePtr, "Resolution: %ux%u\n", frameBuffer.width, frameBuffer.height);
	fprintf(filePtr, "Render time: %s\n", frameTime.c_str());
	fprintf(filePtr, "Bounces: %u; SPP: %u\n", rayTracer.maxDepth, rayTracer.samplesPerPixel);

	fclose(filePtr);
}

std::string RayTracerDebug::FormatTime(float time)
{
	const uint32_t minute = 60;
	const uint32_t hour = minute * 60;

	uint32_t frameHours = (uint32_t) std::floor(time / hour);
	time -= frameHours * hour;

	uint32_t frameMinutes = (uint32_t)std::floor(time / minute);
	time -= frameMinutes * minute;

	uint32_t frameSeconds = (uint32_t)std::floor(time);
	time -= frameSeconds;

	uint32_t frameMillis = (uint32_t) (time * 1000);

	sprintf(textBuffer, "%u:%02u:%02u", frameHours, frameMinutes, frameSeconds);

	return std::string(textBuffer);
}