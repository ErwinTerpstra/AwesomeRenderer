#include "stdafx.h"

#include "raytracerdebug.h"

#include "inputmanager.h"
#include "raytracer.h"

#include "node.h"
#include "texturefactory.h"
#include "sampler.h"
#include "transformation.h"
#include "textmesh.h"
#include "phongmaterial.h"
#include "model.h"
#include "context.h"
#include "surfaceintegrator.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

RayTracerDebug::RayTracerDebug(Context& context, RayTracer& rayTracer) : context(context), rayTracer(rayTracer), inputManager(InputManager::Instance()), currentIntegrator(0), textBuffer(NULL)
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
	textBuffer = new char[256];

	SetupDebugDisplay();
}

void RayTracerDebug::Update()
{
	if (inputManager.GetKeyDown('G'))
	{
		currentIntegrator = (currentIntegrator + 1) % INTEGRATOR_COUNT;
		rayTracer.currentIntegrator = integrators[currentIntegrator];
	}

	bool plus = inputManager.GetKeyDown(VK_OEM_PLUS);
	bool minus = inputManager.GetKeyDown(VK_OEM_MINUS);
	bool shift = inputManager.GetKey(VK_SHIFT);

	if (plus || minus)
	{
		if (shift)
		{
			if (plus)
				rayTracer.monteCarloIntegrator.sampleCount <<= 1;
			else if (rayTracer.monteCarloIntegrator.sampleCount > 1)
				rayTracer.monteCarloIntegrator.sampleCount >>= 1;

			printf("[AwesomeRenderer]: Settings raytracer sample count to %d\n", rayTracer.monteCarloIntegrator.sampleCount);
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
	sprintf(textBuffer, "Bounces: %u; SPP: %u", rayTracer.maxDepth, rayTracer.monteCarloIntegrator.sampleCount);
	debugText->SetText(textBuffer);
}