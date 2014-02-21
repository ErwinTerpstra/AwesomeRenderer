#include <memory.h>
#include <float.h>

#include "awesomerenderer.h"

using namespace AwesomeRenderer;


SoftwareRenderer::SoftwareRenderer() : Renderer()
{

}

SoftwareRenderer::~SoftwareRenderer()
{

}

void SoftwareRenderer::Render()
{
	std::vector<Node*>::const_iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
		DrawModel((*it)->model, (*it)->transform);
}

void SoftwareRenderer::DrawModel(const Model& model, Transformation& trans)
{
	// Iterate through meshes in the model
	for (unsigned int cMesh = 0; cMesh < model.meshes.size(); ++cMesh)
	{
		const Mesh& mesh = *model.meshes[cMesh];
		const Material& material = *model.materials[cMesh];

		BeginDraw(trans.WorldMtx(), material);

		// Iterate through triangles in the mesh
		for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
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

		EndDraw();
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
	
	for (int cVertex = 0; cVertex < 3; ++cVertex)
	{
		// Retrieve output from vertex shader
		shader->ProcessVertex(vertexBuffer[cVertex], vtp[cVertex]);
	}
	
	for (int cVertex = 0; cVertex < 3; ++cVertex)
	{
		// Retrieve z factor before normalizing, this is used for perspective correct interpolation
		float zRecip = 1.0f / vtp[cVertex].screenPosition[3];

		// Convert to screen (pixel) coordinates
		vtp[cVertex].screenPosition = vtp[cVertex].screenPosition * renderContext->camera->viewportMtx;

		// Normalize coordinates (perspective correction)
		cml::detail::divide_by_w(vtp[cVertex].screenPosition);
		
		// Store z factor in unused W channel
		vtp[cVertex].screenPosition[3] = zRecip;
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
	int minY = (int) std::ceil(sst[0][1]), maxY = (int) std::floor(sst[2][1]);
	for (int y = std::max(minY, 0); y <= std::min(maxY, frameBuffer->height - 1); ++y)
	{	
		float x1 = _finite(dAC) ? sst[0][0] + (y - sst[0][1]) * dAC : sst[2][0];
		float x2;

		// If we passed vertex B, we must use line BC instead of AB
		if (y > sst[1][1])
			x2 = _finite(dBC) ? sst[1][0] + (y - sst[1][1]) * dBC : sst[2][0];
		else
			x2 = _finite(dAB) ? sst[0][0] + (y - sst[0][1]) * dAB : sst[1][0];
		
		int minX = (int) std::ceil(std::min(x1, x2));
		int maxX = (int) std::floor(std::max(x1, x2));

		// Fill this scan line
		for (int x = std::max(minX, 0); x <= std::min(maxX, frameBuffer->width - 1); ++x)
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
			
			if (depthBuffer != NULL)
			{
				// Depth testing
				float depth = 1.0f - interpolated.screenPosition[2];
	
				if (depthBuffer->GetPixel(x, y) > depth)
					continue;

				// Write to depth buffer
				depthBuffer->SetPixel(x, y, depth);
			}

			// Compute pixel shading
			SoftwareShader::PixelInfo pixelInfo;
			shader->ProcessPixel(interpolated, pixelInfo);
						
			// Write to color buffer
			frameBuffer->SetPixel(x, y, pixelInfo.color);
		}
	}

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
