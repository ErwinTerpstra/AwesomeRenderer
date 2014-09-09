#include <memory.h>
#include <float.h>

#include "awesomerenderer.h"

using namespace AwesomeRenderer;

SoftwareRenderer::SoftwareRenderer() : Renderer(), renderQueue()
{

}

SoftwareRenderer::~SoftwareRenderer()
{

}

void SoftwareRenderer::Render()
{
	std::vector<Node*>::const_iterator it;

	// Iterate through all renderable nodes
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Model* model = (*it)->model;
		const Transformation* trans = (*it)->transform;

		// Iterate through all meshes in the model
		for (uint32_t cMesh = 0; cMesh < model->meshes.size(); ++cMesh)
		{
			const Mesh* mesh = model->meshes[cMesh];
			const Material* material = model->materials[cMesh];

			// Create a render job for this mesh
			RenderJob job;
			job.mesh = mesh;
			job.material = material;
			job.trans = trans;

			// Render translucent objects last
			if (material->translucent)
				renderQueue.push_back(job);
			else
				renderQueue.push_front(job);
		}

	}

	// Render all jobs in the queue
	while (!renderQueue.empty())
	{
		const RenderJob& job = renderQueue.front();
		DrawJob(job);

		renderQueue.pop_front();
	}
}

void SoftwareRenderer::DrawJob(const RenderJob& job)
{
	BeginDraw(job.trans->WorldMtx(), *job.material);
	
	DrawMesh(*job.mesh);

	EndDraw();
}

void SoftwareRenderer::DrawModel(const Model& model, const Transformation& trans)
{
	// Iterate through meshes in the model
	for (uint32_t cMesh = 0; cMesh < model.meshes.size(); ++cMesh)
	{
		BeginDraw(trans.WorldMtx(), *model.materials[cMesh]);

		DrawMesh(*model.meshes[cMesh]);

		EndDraw();
	}
}

void SoftwareRenderer::DrawMesh(const Mesh& mesh)
{

	// Iterate through triangles in the mesh
	for (uint32_t cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		SoftwareShader::VertexInfo vertexBuffer[3];

		// Setup vertex data buffer for rendering
		for (int cVertex = 0; cVertex < 3; ++cVertex)
		{
			int index = mesh.indices[cIndex + cVertex];

			if (mesh.HasAttribute(Mesh::VERTEX_POSITION))
				vertexBuffer[cVertex].position = mesh.vertices[index];

			if (mesh.HasAttribute(Mesh::VERTEX_NORMAL))
				vertexBuffer[cVertex].normal = mesh.normals[index];

			if (mesh.HasAttribute(Mesh::VERTEX_COLOR))
				vertexBuffer[cVertex].color = mesh.colors[index];

			if (mesh.HasAttribute(Mesh::VERTEX_TEXCOORD))
				vertexBuffer[cVertex].uv = mesh.texcoords[index];
		}

		DrawTriangle(vertexBuffer);
	}
}

void SoftwareRenderer::BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode)
{
	currentMaterial = &material;

	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	// Setup geometry matrices for shader
	shader->modelMtx = model;
	shader->viewMtx = renderContext->camera->viewMtx;
	shader->projMtx = renderContext->camera->projMtx;

	shader->viewPosition = Vector4(renderContext->camera->position, 1.0f);

	// Setup shader rendering parameters
	shader->material = &material;

	shader->Prepare();

}

void SoftwareRenderer::EndDraw()
{

}

void SoftwareRenderer::DrawTriangle(const SoftwareShader::VertexInfo* vertexBuffer)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	SoftwareShader::VertexToPixel vtp[3];
	
	for (uint8_t cVertex = 0; cVertex < 3; ++cVertex)
	{
		// Retrieve output from vertex shader
		shader->ProcessVertex(vertexBuffer[cVertex], vtp[cVertex]);
	}
	
	for (uint8_t cVertex = 0; cVertex < 3; ++cVertex)
	{
		SoftwareShader::VertexToPixel& vertexData = vtp[cVertex];

		// Retrieve z factor before normalizing, this is used for perspective correct interpolation
		float wRecip = 1.0f / vertexData.screenPosition[3];

		// Convert to screen (pixel) coordinates
		vertexData.screenPosition = vertexData.screenPosition * renderContext->camera->viewportMtx;

		// Normalize coordinates and vertex attributes (perspective correction)
		vertexData.screenPosition	*= wRecip;
		vertexData.worldPosition	*= wRecip;
		vertexData.color			*= wRecip;
		vertexData.normal			*= wRecip;
		vertexData.uv				*= wRecip;
		
		// Store z factor in unused W channel
		vertexData.screenPosition[3] = wRecip;
	}

	// Check if we can cull this triangle
	if (cullMode != CULL_NONE)
	{
		Vector3 ab = (vtp[1].screenPosition.subvector(3) - vtp[0].screenPosition.subvector(3));
		Vector3 ac = (vtp[2].screenPosition.subvector(3) - vtp[0].screenPosition.subvector(3));
		Vector3 normal = cml::cross(ab, ac);

		if (normal[2] * (int) cullMode > 0.0f)
			return;
	}
	
	SoftwareShader::VertexToPixel* a = &vtp[0];
	SoftwareShader::VertexToPixel* b = &vtp[1];
	SoftwareShader::VertexToPixel* c = &vtp[2];

	// Sort the vertices in Y direction for rasterizing
	SortTriangle(&a, &b, &c);

	// Screen-space triangle for bounds checking
	Triangle2D sst(Vector2(a->screenPosition[0], a->screenPosition[1]), 
				   Vector2(b->screenPosition[0], b->screenPosition[1]),  
				   Vector2(c->screenPosition[0], c->screenPosition[1]));

	sst.PreCalculateBarycentric();

	//
	// Scan line rasterization
	//
	
	float dAB = (sst[1][0] - sst[0][0]) / (sst[1][1] - sst[0][1]);
	float dAC = (sst[2][0] - sst[0][0]) / (sst[2][1] - sst[0][1]);
	float dBC = (sst[2][0] - sst[1][0]) / (sst[2][1] - sst[1][1]);

	// Iterate trough scan lines
	int minY = (int)std::ceil(sst[0][1]), maxY = (int)std::floor(sst[2][1]);
	for (int y = std::max(minY, 0); y <= std::min(maxY, (int) frameBuffer->height - 1); ++y)
	{	
		float x1 = _finite(dAC) ? sst[0][0] + (y - sst[0][1]) * dAC : sst[2][0];
		float x2;

		// If we passed vertex B, we must use line BC instead of AB
		if (y > sst[1][1])
			x2 = _finite(dBC) ? sst[1][0] + (y - sst[1][1]) * dBC : sst[2][0];
		else
			x2 = _finite(dAB) ? sst[0][0] + (y - sst[0][1]) * dAB : sst[1][0];
		
		int minX = (int)std::ceil(std::min(x1, x2));
		int maxX = (int)std::floor(std::max(x1, x2));

		// Fill this scan line
		for (int x = std::max(minX, 0); x <= std::min(maxX, (int) frameBuffer->width - 1); ++x)
		{
			Vector3 bcCoords;
			sst.CalculateBarycentricCoords(Vector2((float) x, (float) y), bcCoords);
			
			// Interpolate pixel data
			SoftwareShader::VertexToPixel interpolated;
			VectorUtil<4>::Interpolate(a->screenPosition,	b->screenPosition,	c->screenPosition,	bcCoords,	interpolated.screenPosition);
			VectorUtil<4>::Interpolate(a->worldPosition,	b->worldPosition,	c->worldPosition,	bcCoords,	interpolated.worldPosition);
			VectorUtil<4>::Interpolate(a->color,			b->color,			c->color,			bcCoords,	interpolated.color);
			VectorUtil<3>::Interpolate(a->normal,			b->normal,			c->normal,			bcCoords,	interpolated.normal);
			VectorUtil<2>::Interpolate(a->uv,				b->uv,				c->uv,				bcCoords,	interpolated.uv);
			
			// Correct for perspective since we interpolate in screen space
			float wRecip = 1.0f / interpolated.screenPosition[3];
			interpolated.worldPosition	*= wRecip;
			interpolated.color			*= wRecip;
			interpolated.normal			*= wRecip;
			interpolated.uv				*= wRecip;

			interpolated.normal.normalize();

			float depth = 1.0f - interpolated.screenPosition[2];
			
			// Depth testing
			if (depthBuffer != NULL && depthBuffer->GetPixel(x, y) > depth)
				continue;

			// Compute pixel shading
			SoftwareShader::PixelInfo pixelInfo;
			shader->ProcessPixel(interpolated, pixelInfo);

			// Check whether we need to alpha blend colors
			bool alphaBlend = pixelInfo.color[3] < 1.0f;
			
			if (alphaBlend)
			{
				Color color;
				frameBuffer->GetPixel(x, y, color);

				Blend(pixelInfo.color, color, color);
				frameBuffer->SetPixel(x, y, color);
			}
			else
			{
				// Write to depth buffer
				if (depthBuffer != NULL)
					depthBuffer->SetPixel(x, y, depth);

				// Write to color buffer
				frameBuffer->SetPixel(x, y, pixelInfo.color);
			}
		}
	}

}

void SoftwareRenderer::Blend(const Color& src, const Color& dst, Color& out)
{
	for (uint8_t channel = 0; channel < 3; ++channel)
		out[channel] = (src[channel] * src[3]) + (dst[channel] * (1.0 - src[3]));

}

void SoftwareRenderer::SortTriangle(SoftwareShader::VertexToPixel** a, SoftwareShader::VertexToPixel** b, SoftwareShader::VertexToPixel** c)
{
	if ((*a)->screenPosition[1] > (*b)->screenPosition[1]) Swap<SoftwareShader::VertexToPixel>(a, b);
	if ((*b)->screenPosition[1] > (*c)->screenPosition[1]) Swap<SoftwareShader::VertexToPixel>(b, c);
	if ((*a)->screenPosition[1] > (*b)->screenPosition[1]) Swap<SoftwareShader::VertexToPixel>(a, b);
}

template <typename T>
void SoftwareRenderer::Swap(T** a, T** b)
{
	T* tmp = *a;
	*a = *b;
	*b = tmp;
}
