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

	GL_CHECK_ERROR(glEnableClientState(GL_VERTEX_ARRAY));

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		Model* model = (*it)->GetComponent<Model>();
		Transformation* transform = (*it)->GetComponent<Transformation>();

		if (model == NULL)
			continue;

		DrawModel(*model, *transform);
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	PostRender();
}

void RendererGL::DrawModel(const Model& model, const Transformation& trans)
{
	// Iterate through meshes in the model
	for (unsigned int cMesh = 0; cMesh < model.meshes.size(); ++cMesh)
	{
		const Mesh& mesh = *model.meshes[cMesh];
		const Material& material = *model.materials[cMesh];

		BeginDraw(trans.WorldMtx(), material);

		MeshGL* meshGL = mesh.As<MeshGL>();
		
		glBindVertexArray(meshGL->vertexArray);

		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, NULL);
		
		glBindVertexArray(0);

		EndDraw();
	}

	GL_CHECK_ERROR("DrawModel");
}

void RendererGL::BeginDraw(const Matrix44& model, const Material& material)
{
	currentMaterial = &material;

	glDepthMask(material.translucent ? GL_FALSE : GL_TRUE);

	ProgramGL* shader = &defaultShader;

	shader->Prepare();

	// Setup geometry matrices for shader
	GL_CHECK_ERROR(glUniformMatrix4fv(shader->GetUniformLocation("modelMtx"), 1, GL_FALSE, model.data()));
	GL_CHECK_ERROR(glUniformMatrix4fv(shader->GetUniformLocation("viewMtx"), 1, GL_FALSE, renderContext->camera->viewMtx.data()));
	GL_CHECK_ERROR(glUniformMatrix4fv(shader->GetUniformLocation("projMtx"), 1, GL_FALSE, renderContext->camera->projMtx.data()));

	// Load textures supplied by material
	GLenum activeTexture = GL_TEXTURE0;

	const PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	if (phongMaterial->diffuseMap != NULL)
		shader->BindTexture(phongMaterial->diffuseMap->texture->As<TextureGL>(), "diffuseMap", activeTexture++);

	if (phongMaterial->normalMap != NULL)
		shader->BindTexture(phongMaterial->normalMap->texture->As<TextureGL>(), "normalMap", activeTexture++);

	if (phongMaterial->specularMap != NULL)
		shader->BindTexture(phongMaterial->specularMap->texture->As<TextureGL>(), "specularMap", activeTexture++);

}

void RendererGL::EndDraw()
{

}

void RendererGL::Cleanup()
{

}