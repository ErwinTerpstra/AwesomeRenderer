#include "awesomerenderer.h"

using namespace AwesomeRenderer;

GLRenderer::GLRenderer(GLWindow& window) : Renderer(), window(window), 
	defaultShader(),
	defaultVertex(GL_VERTEX_SHADER),
	defaultFragment(GL_FRAGMENT_SHADER)
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

	defaultShader.Link();
}

void GLRenderer::Render()
{
	glClearColor(0.4f, 0.6f, 0.9f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear required buffers  

	glEnableClientState(GL_VERTEX_ARRAY);

	std::vector<Node*>::const_iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
		DrawModel(*(*it)->model, *(*it)->transform);

	glDisableClientState(GL_VERTEX_ARRAY);

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

		MeshGL* meshGL = mesh.As<MeshGL>();


		// Vertices
		glBindBuffer(GL_ARRAY_BUFFER, meshGL->vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		// Indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGL->indexBuffer);

		// Draw call
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, NULL);

		EndDraw();
	}
}

void GLRenderer::BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode)
{
	currentMaterial = &material;

	GLProgram* shader = &defaultShader;

	// Setup geometry matrices for shader
	//shader->modelMtx = model;
	//shader->viewMtx = renderContext->camera->viewMtx;
	//shader->projMtx = renderContext->camera->projMtx;

	// Setup shader rendering parameters
	//shader->material = &material;

	shader->Bind();
}

void GLRenderer::EndDraw()
{

}
