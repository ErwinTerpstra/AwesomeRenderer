#include "awesomerenderer.h"

using namespace AwesomeRenderer;

RendererGL::RendererGL(WindowGL& window) : Renderer(), window(window), 
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

	defaultShader.Link();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void RendererGL::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	std::vector<Node*>::const_iterator it;

	glEnableClientState(GL_VERTEX_ARRAY);

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
		DrawModel(*(*it)->model, *(*it)->transform);

	glDisableClientState(GL_VERTEX_ARRAY);
	
	window.Draw();
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
}

void RendererGL::BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode)
{
	currentMaterial = &material;

	ProgramGL* shader = &defaultShader;
	
	// Setup geometry matrices for shader
	glUniformMatrix4fv(shader->GetUniformLocation("modelMtx"), 1, GL_FALSE, model.data());
	glUniformMatrix4fv(shader->GetUniformLocation("viewMtx"), 1, GL_FALSE, renderContext->camera->viewMtx.data());
	glUniformMatrix4fv(shader->GetUniformLocation("projMtx"), 1, GL_FALSE, renderContext->camera->projMtx.data());
	
	// Load textures supplied by material
	GLenum activeTexture = GL_TEXTURE0;
	if (material.diffuseMap != NULL)
		shader->BindTexture(material.diffuseMap->texture->As<TextureGL>(), "diffuseMap", activeTexture++);

	if (material.normalMap != NULL)
		shader->BindTexture(material.normalMap->texture->As<TextureGL>(), "normalMap", activeTexture++);

	if (material.specularMap != NULL)
		shader->BindTexture(material.specularMap->texture->As<TextureGL>(), "specularMap", activeTexture++);

	shader->Prepare();
}

void RendererGL::EndDraw()
{

}