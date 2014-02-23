#include "awesomerenderer.h"

using namespace AwesomeRenderer;

GLRenderer::GLRenderer(GLWindow& window) : Renderer(), window(window)
{
	
}

void GLRenderer::Render()
{
	glClearColor(0.4f, 0.6f, 0.9f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers  

	std::vector<Node*>::const_iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
		DrawModel(*(*it)->model, *(*it)->transform);

	window.Draw();
}

void GLRenderer::DrawModel(const Model& model, const Transformation& trans)
{
	// Iterate through meshes in the model
	for (unsigned int cMesh = 0; cMesh < model.meshes.size(); ++cMesh)
	{
		const Mesh& mesh = *model.meshes[cMesh];
		const Material& material = *model.materials[cMesh];

		BeginDraw(trans.WorldMtx(), material);



		EndDraw();
	}
}

void GLRenderer::BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode)
{
	currentMaterial = &material;

	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	// Setup geometry matrices for shader
	shader->modelMtx = model;
	shader->viewMtx = renderContext->camera->viewMtx;
	shader->projMtx = renderContext->camera->projMtx;

	// Setup shader rendering parameters
	shader->material = &material;

	shader->Prepare();

}

void GLRenderer::EndDraw()
{

}
