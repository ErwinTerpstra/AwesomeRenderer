#include "stdafx.h"
#include "renderer_gl.h"
#include "filereader.h"
#include "rendercontext.h"
#include "window.h"
#include "material.h"
#include "phongmaterial.h"
#include "sampler.h"
#include "texture.h"
#include "camera.h"

#include "node.h"
#include "model.h"
#include "mesh.h"
#include "mesh_gl.h"
#include "transformation.h"

#include "util_gl.h"
#include "window_gl.h"
#include "texture_gl.h"
#include "rendertarget_gl.h"

using namespace AwesomeRenderer;

RendererGL::RendererGL() : Renderer(),
	defaultShader(),
	defaultVertex(GL_VERTEX_SHADER),
	defaultFragment(GL_FRAGMENT_SHADER)
{
	opaque.sortMode = FRONT_TO_BACK;
	transparent.sortMode = BACK_TO_FRONT;
}


void RendererGL::Initialize()
{
	FileReader fileReader;

	char buffer[1024 * 128];

	// Read vertex shader
	fileReader.Open("../Assets/vertex.glsl");
	fileReader.Read(&buffer[0]);
	fileReader.Close();
	
	// Compile vertex shader
	const char *vertexShaderSrc[] = { buffer };
	defaultVertex.Compile(vertexShaderSrc, 1);

	// Read fragment shader
	fileReader.Open("../Assets/fragment.glsl");
	fileReader.Read(&buffer[0]);
	fileReader.Close();
	
	// Compile fragment shader
	const char *fragmentShaderSrc[] = { buffer };
	defaultFragment.Compile(fragmentShaderSrc, 1);

	// Attach shaders to program
	defaultShader.Attach(&defaultVertex);
	defaultShader.Attach(&defaultFragment);

	defaultShader.SetAttribLocation("inPosition",	MeshGL::ATTR_POSITION);
	defaultShader.SetAttribLocation("inNormal",		MeshGL::ATTR_NORMAL);
	defaultShader.SetAttribLocation("inColor",		MeshGL::ATTR_COLOR);
	defaultShader.SetAttribLocation("inTexcoord",	MeshGL::ATTR_TEXCOORD);

	defaultShader.Link();

	GL_CHECK_ERROR(
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

}

void RendererGL::PreRender()
{
	RenderTargetGL* renderTargetGL = renderContext->renderTarget->As<RenderTargetGL>();

	if (renderTargetGL != NULL)
		renderTargetGL->Bind();

	uint32_t clearBits = 0;

	if ((renderContext->clearFlags & RenderTarget::BUFFER_COLOR) != 0)
		clearBits |= GL_COLOR_BUFFER_BIT;

	if ((renderContext->clearFlags & RenderTarget::BUFFER_DEPTH) != 0)
		clearBits |= GL_DEPTH_BUFFER_BIT;

	glClear(clearBits);

	if (cullMode != CULL_NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GetCullMode());
	}
	else
		glDisable(GL_CULL_FACE);

	GL_CHECK_ERROR("PreRender");
}

void RendererGL::PostRender()
{
	RenderTargetGL* renderTargetGL = renderContext->renderTarget->As<RenderTargetGL>();

	if (renderTargetGL != NULL)
	{
		renderTargetGL->Read();
		renderTargetGL->Unbind();
	}

	GL_CHECK_ERROR("PostRender");
}

void RendererGL::Present(Window& window)
{
	WindowGL* windowGL = window.As<WindowGL>();

	if (windowGL != NULL)
		windowGL->Draw();
}

void RendererGL::Render()
{
	PreRender();

	std::vector<Node*>::const_iterator it;
	
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		Model* model = (*it)->GetComponent<Model>();
		Transformation* transform = (*it)->GetComponent<Transformation>();

		if (model == NULL)
			continue;

		EnqueueModel(*model, *transform);
	}
	
	SortRenderQueue(opaque);
	SortRenderQueue(transparent);

	GL_CHECK_ERROR(glEnableClientState(GL_VERTEX_ARRAY));

	DrawRenderQueue(opaque);
	DrawRenderQueue(transparent);

	glDisableClientState(GL_VERTEX_ARRAY);

	PostRender();
}

void RendererGL::SortRenderQueue(RenderQueue& queue)
{
	// TODO: sort render jobs by material
	JobComparator comparator(renderContext->camera->position, queue.sortMode);

	std::sort(queue.jobs.begin(), queue.jobs.end(), comparator);
}

RendererGL::JobComparator::JobComparator(const Vector3& cameraPosition, SortMode sortMode) :
	cameraPosition(cameraPosition), sortMode(sortMode)
{

}

bool RendererGL::JobComparator::operator()(const RenderJob& a, const RenderJob& b)
{
	Vector3 pA = a.trans->GetWorldPosition();
	Vector3 pB = b.trans->GetWorldPosition();

	float dA = (cameraPosition - pA).length();
	float dB = (cameraPosition - pB).length();

	switch (sortMode)
	{
	default:
	case BACK_TO_FRONT:
		return dA > dB;

	case FRONT_TO_BACK:
		return dA < dB;
	}
}

void RendererGL::DrawRenderQueue(RenderQueue& queue)
{
	// Render all jobs in the queue
	while (!queue.jobs.empty())
	{
		const RenderJob& job = queue.jobs.front();

		DrawJob(job);

		queue.jobs.pop_front();
	}

}

void RendererGL::EnqueueModel(const Model& model, const Transformation& trans)
{
	// Iterate through meshes in the model
	for (unsigned int cMesh = 0; cMesh < model.meshes.size(); ++cMesh)
	{
		const Mesh* mesh = model.meshes[cMesh];
		const Material* material = model.materials[cMesh];

		RenderJob job;
		job.mesh = mesh;
		job.material = material;
		job.trans = &trans;
		
		// Render translucent objects in a separate queue
		if (material->translucent)
			transparent.jobs.push_back(job);
		else
			opaque.jobs.push_front(job);		
	}

}

void RendererGL::DrawJob(const RenderJob& job)
{
	MeshGL* meshGL = job.mesh->As<MeshGL>();

	if (meshGL == NULL)
		return;

	BeginDraw(job.trans->WorldMtx(), *job.material);

	glBindVertexArray(meshGL->vertexArray);

	glDrawElements(GL_TRIANGLES, job.mesh->indices.size(), GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);

	EndDraw();

	GL_CHECK_ERROR("DrawJob");
}

void RendererGL::BeginDraw(const Matrix44& model, const Material& material)
{
	glDepthMask(material.translucent ? GL_FALSE : GL_TRUE);

	ProgramGL* shader = &defaultShader;

	shader->Prepare();

	// Setup geometry matrices for shader
	shader->SetMatrix44("modelMtx", model);
	shader->SetMatrix44("viewMtx", renderContext->camera->viewMtx);
	shader->SetMatrix44("projMtx", renderContext->camera->projMtx);

	const PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	shader->SetVector4("diffuseColor", phongMaterial->diffuseColor);
	shader->SetVector4("specularColor", phongMaterial->specularColor);

	// Load textures supplied by material
	GLenum activeTexture = GL_TEXTURE0;

	TextureGL* diffuseTexture = phongMaterial->diffuseMap != NULL ? phongMaterial->diffuseMap->texture->As<TextureGL>() : NULL;
	shader->BindTexture(diffuseTexture, "diffuseMap", activeTexture++);
	
	TextureGL* normalTexture = phongMaterial->normalMap != NULL ? phongMaterial->normalMap->texture->As<TextureGL>() : NULL;
	shader->BindTexture(normalTexture, "normalMap", activeTexture++);

	TextureGL* specularTexture = phongMaterial->specularMap != NULL ? phongMaterial->specularMap->texture->As<TextureGL>() : NULL;
	shader->BindTexture(specularTexture, "specularMap", activeTexture++);
}

void RendererGL::EndDraw()
{
	glDepthMask(GL_TRUE);
}

void RendererGL::Cleanup()
{

}