#include "awesomerenderer.h"

using namespace AwesomeRenderer;

SoftwareRenderer::SoftwareRenderer() : Renderer(), renderQueue(), tileIdx(0), tilesLeft(), workerSignal(0, WORKER_AMOUNT), mainThreadSignal(0, 1)
{

}

SoftwareRenderer::~SoftwareRenderer()
{

}

void SoftwareRenderer::Initialize()
{
	for (uint32_t workerIdx = 0; workerIdx < WORKER_AMOUNT; ++workerIdx)
	{	
		WorkerThread& thread = workers[workerIdx];

		WorkerData data;
		data.renderer = this;
		data.thread = &thread;

		thread.Start(data);

		// Wait for the thread to signal it is finished setting up
		// This is neccesary since the worker creation accesses local stack data
		mainThreadSignal.Wait();
	}
}

void SoftwareRenderer::Cleanup()
{
	// Close all created threads
	for (uint32_t workerIdx = 0; workerIdx < WORKER_AMOUNT; ++workerIdx)
	{
		WorkerThread& thread = workers[workerIdx];
		thread.Stop();

		// Wait for the thread to signal it has been shut down
		mainThreadSignal.Wait();
	}
}

void SoftwareRenderer::SetRenderContext(const RenderContext* context)
{
	Renderer::SetRenderContext(context);

	Buffer* frameBuffer = context->renderTarget->frameBuffer;

	// Calculate how many tiles we need for this frame buffer size
	horizontalTiles = (frameBuffer->width + TILE_WIDTH - 1) / TILE_WIDTH;
	verticalTiles = (frameBuffer->height + TILE_HEIGHT - 1) / TILE_HEIGHT;

	uint32_t tileAmount = horizontalTiles * verticalTiles;

	// Resize our tiles vector to fit all tile bins on screen
	tiles.resize(tileAmount);

	tilesLeft.Configure(0, tileAmount);
}

void SoftwareRenderer::PreRender()
{
	renderContext->renderTarget->Clear(Color::BLACK);
}

void SoftwareRenderer::PostRender()
{
	GdiBuffer* buffer = static_cast<GdiBuffer*>(renderContext->renderTarget->frameBuffer);

	if (renderContext->window != NULL && buffer != NULL)
		renderContext->window->DrawBuffer(*buffer);
}

void SoftwareRenderer::Render()
{
	PreRender();

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

			// TODO: sort render jobs by material
			
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
		DrawTiles();
		
		renderQueue.pop_front();
	}

	PostRender();
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

void SoftwareRenderer::BeginDraw(const Matrix44& model, const Material& material)
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
	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	TriangleData data;
	SoftwareShader::VertexToPixel* vtp = data.vertexToPixel;

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
		vertexData.screenPosition		*= wRecip;
		vertexData.worldPosition		*= wRecip;
		vertexData.color				*= wRecip;
		vertexData.normal				*= wRecip;
		vertexData.uv					*= wRecip;

		// Store z factor in unused W channel
		vertexData.screenPosition[3] = wRecip;
	}

	// Check if we can cull this triangle
	if (cullMode != CULL_NONE)
	{
		Vector3 ab = (vtp[1].screenPosition.subvector(3) - vtp[0].screenPosition.subvector(3));
		Vector3 ac = (vtp[2].screenPosition.subvector(3) - vtp[0].screenPosition.subvector(3));
		Vector3 normal = cml::cross(ab, ac);

		if (normal[2] * (int)cullMode > 0.0f)
			return;
	}

	// Sort the vertices for rasterizing
	// y direction for FILL mode and x direction for LINE mode
	switch (drawMode)
	{
	case DRAW_LINE: SortTriangle(vtp, 0);
	case DRAW_FILL: SortTriangle(vtp, 1);
	}

	Triangle2D* sst = &data.screenSpaceTriangle;

	// Screen-space triangle for bounds checking
	*sst = Triangle2D(Vector2(vtp[0].screenPosition[0], vtp[0].screenPosition[1]),
					  Vector2(vtp[1].screenPosition[0], vtp[1].screenPosition[1]),
					  Vector2(vtp[2].screenPosition[0], vtp[2].screenPosition[1]));

	sst->PreCalculateBarycentric();

	// Calculate bounds for the triangle so that we can decide which tiles it is in
	Vector2 lower, upper;
	data.screenSpaceTriangle.CalculateBounds(lower, upper);
	
	// Iterate through all the tiles this triangle intersects with and save it for that tile
	for (int tileY = std::max((int)lower[1] / TILE_HEIGHT, 0); tileY <= std::min((int)upper[1] / TILE_HEIGHT, (int)verticalTiles - 1); ++tileY)
	{
		for (int tileX = std::max((int) lower[0] / TILE_WIDTH, 0); tileX <= std::min((int) upper[0] / TILE_WIDTH, (int) horizontalTiles - 1); ++tileX)
		{
			int idx = (tileY * horizontalTiles) + tileX;
			tiles[idx].push_back(data);
		}
	}
}

void SoftwareRenderer::DrawTiles()
{
	// Reset the rendered tiles counter
	tilesLeft.Reset();

	// Set tile pointer to last tile
	tileIdx.Lock();
	*tileIdx = tiles.size();
	tileIdx.Unlock();

	// Notify worker threads that there are tiles to be rendered
	workerSignal.Signal(WORKER_AMOUNT);

	// Wait for all tiles to be rendered
	tilesLeft.WaitZero();
}

void SoftwareRenderer::DrawTile(uint32_t tileX, uint32_t tileY)
{
	switch (drawMode)
	{
	case DRAW_LINE:		DrawTileLine(tileX, tileY);
	case DRAW_FILL:		DrawTileFill(tileX, tileY);
	}
}

void SoftwareRenderer::DrawTileFill(uint32_t tileX, uint32_t tileY)
{
	std::vector<TriangleData>& bin = tiles[(tileY * horizontalTiles) + tileX];

	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	// Retrieve min and max screen coordinates for this tile
	int32_t minTileX = std::max(tileX * TILE_WIDTH, 0u);
	int32_t minTileY = std::max(tileY * TILE_HEIGHT, 0u);

	int32_t maxTileX = std::min(minTileX + TILE_WIDTH - 1, (int32_t)frameBuffer->width - 1);
	int32_t maxTileY = std::min(minTileY + TILE_HEIGHT - 1, (int32_t)frameBuffer->height - 1);

	// Draw all triangles in the tile
	while (!bin.empty())
	{
		const TriangleData& data = bin.back();

		const Triangle2D& sst = data.screenSpaceTriangle;
		const SoftwareShader::VertexToPixel* a = &data.vertexToPixel[0];
		const SoftwareShader::VertexToPixel* b = &data.vertexToPixel[1];
		const SoftwareShader::VertexToPixel* c = &data.vertexToPixel[2];

		//
		// Scan line rasterization
		//

		float dAB = (sst[1][0] - sst[0][0]) / (sst[1][1] - sst[0][1]);
		float dAC = (sst[2][0] - sst[0][0]) / (sst[2][1] - sst[0][1]);
		float dBC = (sst[2][0] - sst[1][0]) / (sst[2][1] - sst[1][1]);

		// Iterate trough scan lines
		int32_t minY = (int32_t)std::ceil(sst[0][1]), maxY = (int32_t)std::floor(sst[2][1]);
		for (int32_t y = std::max(minY, minTileY); y <= std::min(maxY, maxTileY); ++y)
		{
			float x1 = _finite(dAC) ? sst[0][0] + (y - sst[0][1]) * dAC : sst[2][0];
			float x2;

			// If we passed vertex B, we must use line BC instead of AB
			if (y > sst[1][1])
				x2 = _finite(dBC) ? sst[1][0] + (y - sst[1][1]) * dBC : sst[2][0];
			else
				x2 = _finite(dAB) ? sst[0][0] + (y - sst[0][1]) * dAB : sst[1][0];

			int32_t minX = (int32_t)std::ceil(std::min(x1, x2));
			int32_t maxX = (int32_t)std::floor(std::max(x1, x2));

			// Fill this scan line
			for (int x = std::max(minX, minTileX); x <= std::min(maxX, maxTileX); ++x)
			{
				Vector3 bcCoords;
				sst.CalculateBarycentricCoords(Vector2((float)x, (float)y), bcCoords);

				// Interpolate pixel data
				SoftwareShader::VertexToPixel interpolated;
				VectorUtil<4>::Interpolate(a->screenPosition, b->screenPosition, c->screenPosition, bcCoords, interpolated.screenPosition);
				VectorUtil<4>::Interpolate(a->worldPosition, b->worldPosition, c->worldPosition, bcCoords, interpolated.worldPosition);
				VectorUtil<4>::Interpolate(a->color, b->color, c->color, bcCoords, interpolated.color);
				VectorUtil<3>::Interpolate(a->normal, b->normal, c->normal, bcCoords, interpolated.normal);
				VectorUtil<2>::Interpolate(a->uv, b->uv, c->uv, bcCoords, interpolated.uv);

				// Correct for perspective since we interpolate in screen space
				float wRecip = 1.0f / interpolated.screenPosition[3];
				interpolated.worldPosition *= wRecip;
				interpolated.color *= wRecip;
				interpolated.normal *= wRecip;
				interpolated.uv *= wRecip;

				interpolated.normal.normalize();

				float depth = 1.0f - interpolated.screenPosition[2];

				// Depth testing
				if (depthBuffer != NULL && depthBuffer->GetPixel(x, y) > depth)
					continue;

				// Compute pixel shading
				SoftwareShader::PixelInfo pixelInfo;
				shader->ProcessPixel(interpolated, pixelInfo);

				// Check whether we need to alpha blend colors
				if (currentMaterial->translucent)
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

		bin.pop_back();
	}
}

void SoftwareRenderer::DrawTileLine(uint32_t tileX, uint32_t tileY)
{
	std::vector<TriangleData>& bin = tiles[(tileY * horizontalTiles) + tileX];
	
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	// Retrieve min and max screen coordinates for this tile
	int32_t minTileX = std::max(tileX * TILE_WIDTH, 0u);
	int32_t minTileY = std::max(tileY * TILE_HEIGHT, 0u);

	int32_t maxTileX = std::min(minTileX + TILE_WIDTH - 1, (int32_t)frameBuffer->width - 1);
	int32_t maxTileY = std::min(minTileY + TILE_HEIGHT - 1, (int32_t)frameBuffer->height - 1);

	// Draw all triangles in the tile
	while (!bin.empty())
	{
		const TriangleData& data = bin.back(); 

		const Triangle2D& sst = data.screenSpaceTriangle;
		const SoftwareShader::VertexToPixel* a = &data.vertexToPixel[0];
		const SoftwareShader::VertexToPixel* b = &data.vertexToPixel[1];
		const SoftwareShader::VertexToPixel* c = &data.vertexToPixel[2];

		int32_t points[] =
		{
			sst[0][0], sst[0][1], sst[1][0], sst[1][1],
			sst[0][0], sst[0][1], sst[2][0], sst[2][1],
			sst[1][0], sst[1][1], sst[2][0], sst[2][1],
		};

		//
		// Bresenham's_line_algorithm
		
		for (uint32_t lineIdx = 0; lineIdx < 3; ++lineIdx)
		{
			uint32_t b = lineIdx * 4;

			int32_t x1 = points[b + 0];
			int32_t y1 = points[b + 1];
			int32_t x2 = points[b + 2];
			int32_t y2 = points[b + 3];

			const bool steep = (abs(y2 - y1) > abs(x2 - x1));

			// Swap coordinate system if slope is bigger than 45 degrees
			if (steep)
			{
				Util::Swap(x1, y1);
				Util::Swap(x2, y2);
			}

			// Make sure x1 is smaller than x2
			if (x1 > x2)
			{
				Util::Swap(x1, x2);
				Util::Swap(y1, y2);
			}

			int32_t deltaX = x2 - x1;
			int32_t deltaY = y2 - y1;

			float error = 0.0f;
			float deltaError = abs((float) deltaY / deltaX);
			int32_t yStep = y1 < y2 ? 1 : -1;

			int32_t y = y1;

			for (int32_t x = x1; x <= x2; ++x)
			{
				int32_t screenX = x;
				int32_t screenY = y;

				if (steep)
					Util::Swap(screenX, screenY);

				if (screenX >= minTileX && screenY >= minTileY && screenX <= maxTileX && screenY <= maxTileY)
					frameBuffer->SetPixel(screenX, screenY, Color::WHITE);

				error += deltaError;

				if (error >= 0.5f)
				{
					y += yStep;
					error -= 1.0f;
				}
			}

		}

		bin.pop_back();
	}
}


DWORD SoftwareRenderer::StartWorker(WorkerThread* thread)
{
	// Signal the main thread that we finished setting up
	mainThreadSignal.Signal();

	while (thread->IsRunning())
	{
		// Wait until the tiles are ready to be rendered
		workerSignal.Wait();

		while (thread->IsRunning())
		{
			bool hasTile = false;

			// Wait until we have access to the tile index counter
			tileIdx.Lock();
			
			// Check if there are tiles left to render
			if (*tileIdx > 0)
			{
				int selectedIdx = (*tileIdx) - 1;
				--(*tileIdx);

				thread->tileX = selectedIdx % horizontalTiles;
				thread->tileY = selectedIdx / horizontalTiles;
				hasTile = true;
			}

			tileIdx.Unlock();

			// No more tiles to render, break inner loop and wait until next frame
			if (!hasTile)
				break;

			// Render the current tile
			DrawTile(thread->tileX, thread->tileY);
			
			tilesLeft.Decrement();
		}
	}

	// Signal the main thread that we finished shutting down
	mainThreadSignal.Signal();

	
	return 0;
}

SoftwareRenderer::WorkerThread::WorkerThread() : available(false), running(false), tileX(0), tileY(0)
{

}

void SoftwareRenderer::WorkerThread::Start(WorkerData& data)
{
	running = true;
	
	// Create and start a thread for this worker
	handle = CreateThread(NULL, 0, SoftwareRenderer::HandleWorker, &data, 0, &id);
}

void SoftwareRenderer::WorkerThread::Stop()
{
	// Signal we are shutting down
	running = false;
	available = false;
}

void SoftwareRenderer::Blend(const Color& src, const Color& dst, Color& out)
{
	for (uint8_t channel = 0; channel < 3; ++channel)
		out[channel] = (src[channel] * src[3]) + (dst[channel] * (1.0 - src[3]));

}

void SoftwareRenderer::SortTriangle(SoftwareShader::VertexToPixel* vtp, uint32_t axis)
{
	if (vtp[0].screenPosition[axis] > vtp[1].screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(vtp[0], vtp[1]);

	if (vtp[1].screenPosition[axis] > vtp[2].screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(vtp[1], vtp[2]);

	if (vtp[0].screenPosition[axis] > vtp[1].screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(vtp[0], vtp[1]);
}

void SoftwareRenderer::SortTriangle(SoftwareShader::VertexToPixel** a, SoftwareShader::VertexToPixel** b, SoftwareShader::VertexToPixel** c, uint32_t axis)
{
	if ((*a)->screenPosition[axis] > (*b)->screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(a, b);

	if ((*b)->screenPosition[axis] > (*c)->screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(b, c);

	if ((*a)->screenPosition[axis] > (*b)->screenPosition[axis])
		Util::Swap<SoftwareShader::VertexToPixel>(a, b);
}
