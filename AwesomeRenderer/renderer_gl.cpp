
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

#include "program_gl.h"

#include "inputmanager.h"


GLfloat quadVertices[] =
{
	// Positions   // TexCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};

using namespace AwesomeRenderer;

RendererGL::RendererGL() : Renderer(),
	defaultShader(), imageVertex(GL_VERTEX_SHADER), imageFragment(GL_FRAGMENT_SHADER)
{
	opaque.sortMode = FRONT_TO_BACK;
	transparent.sortMode = BACK_TO_FRONT;
}


void RendererGL::Initialize()
{
	FileReader fileReader;

	char vertexShaderSource[1024 * 128];
	char fragmentShaderSource[1024 * 128];

	// Read vertex shader
	fileReader.Open("../Assets/Shaders/default_vertex.glsl");
	fileReader.Read(&vertexShaderSource[0]);
	fileReader.Close();

	// Read fragment shader
	fileReader.Open("../Assets/Shaders/default_fragment.glsl");
	fileReader.Read(&fragmentShaderSource[0]);
	fileReader.Close();
	
	defaultShader.SetSource(vertexShaderSource, fragmentShaderSource);

	// Read vertex shader
	fileReader.Open("../Assets/Shaders/image_vertex.glsl");
	fileReader.Read(&vertexShaderSource[0]);
	fileReader.Close();

	// Read fragment shader
	fileReader.Open("../Assets/Shaders/image_fragment.glsl");
	fileReader.Read(&fragmentShaderSource[0]);
	fileReader.Close();

	char* vertexSource = &vertexShaderSource[0];
	char* fragmentSource = &fragmentShaderSource[0];
	imageVertex.Compile((const char**) &vertexSource, 1);
	imageFragment.Compile((const char**) &fragmentSource, 1);

	imageShader.Attach(&imageVertex);
	imageShader.Attach(&imageFragment);
	imageShader.Link();

	/**/
	glGenVertexArrays(1, &quadArray);
	glGenBuffers(1, &quadBuffer);
	
	glBindVertexArray(quadArray);
	
	glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(GLfloat)));

	glBindVertexArray(0);
	/**/


}

void RendererGL::PreRender()
{
	RenderTargetGL* renderTargetGL = renderContext->renderTarget->As<RenderTargetGL>();

	if (renderTargetGL != NULL)
		renderTargetGL->Bind();

	glViewport(0, 0, renderContext->renderTarget->frameBuffer->width, renderContext->renderTarget->frameBuffer->height);

	GL_CHECK_ERROR(
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	);

	uint32_t clearBits = 0;

	if ((renderContext->clearFlags & RenderTarget::BUFFER_COLOR) != 0)
		clearBits |= GL_COLOR_BUFFER_BIT;

	if ((renderContext->clearFlags & RenderTarget::BUFFER_DEPTH) != 0)
		clearBits |= GL_DEPTH_BUFFER_BIT;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

void RendererGL::DrawImage(TextureGL& texture, uint32_t width, uint32_t height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, width, height);

	imageShader.Prepare();
	imageShader.BindTexture(&texture, "image", GL_TEXTURE0);
	
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindVertexArray(quadArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDisableClientState(GL_VERTEX_ARRAY);
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

	const PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	TextureGL* diffuseTexture = phongMaterial->diffuseMap != NULL ? phongMaterial->diffuseMap->texture->As<TextureGL>() : NULL;
	TextureGL* specularTexture = phongMaterial->specularMap != NULL ? phongMaterial->specularMap->texture->As<TextureGL>() : NULL;
	TextureGL* normalTexture = material.normalMap != NULL && !InputManager::Instance().GetKey('N') ? material.normalMap->texture->As<TextureGL>() : NULL;

	// Select the correct shader branch
	defaultShader.SetKeyword("USE_NORMAL_MAP", normalTexture != NULL);
	defaultShader.SetKeyword("OUTPUT_LINEAR", renderContext->renderTarget->frameBuffer->colorSpace == Buffer::LINEAR);

	ProgramGL* shader = defaultShader.GetCurrentBranch();

	shader->Prepare();

	// Setup geometry matrices for shader
	shader->SetMatrix44("modelMtx", model);
	shader->SetMatrix44("viewMtx", renderContext->camera->viewMtx);
	shader->SetMatrix44("projMtx", renderContext->camera->projMtx);

	shader->SetVector3("cameraPosition", renderContext->camera->position);

	// Material properties
	shader->SetVector4("diffuseColor", phongMaterial->diffuseColor);
	shader->SetVector4("specularColor", phongMaterial->specularColor);

	// Load textures supplied by material
	GLenum activeTexture = GL_TEXTURE0;
	
	shader->BindTexture(diffuseTexture, "diffuseMap", activeTexture++);	
	shader->BindTexture(normalTexture, "normalMap", activeTexture++);
	shader->BindTexture(specularTexture, "specularMap", activeTexture++);
}

void RendererGL::EndDraw()
{
	glDepthMask(GL_TRUE);
}

void RendererGL::Cleanup()
{

}