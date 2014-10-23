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
	renderContext->renderTarget->Clear(Color::BLACK, renderContext->clearFlags);
}

void SoftwareRenderer::PostRender()
{

}

void SoftwareRenderer::Present(Window& window)
{
	GdiBuffer* buffer = static_cast<GdiBuffer*>(renderContext->renderTarget->frameBuffer);

	if (buffer != NULL)
		window.DrawBuffer(*buffer);
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

	SoftwareShader::VertexToPixel vtp[3];

	for (uint8_t cVertex = 0; cVertex < 3; ++cVertex)
	{
		// Retrieve output from vertex shader
		shader->ProcessVertex(vertexBuffer[cVertex], vtp[cVertex]);
	}

	// For each vertex, check if it is inside the view frustum
	uint8_t vertices[3];
	uint8_t clippedVertices = 0;
	for (uint8_t cVertex = 0; cVertex < 3; ++cVertex)
	{
		Vector4& v = vtp[cVertex].screenPosition;

		// X and Y axis should be between -W and +W, Z should be between 0 and +W
		// Otherwise vertex is outside the view frustum
		bool inside = -v[3] < v[0] && v[0] < v[3] &&
					  -v[3] < v[1] && v[1] < v[3] &&
					      0 < v[2] && v[2] < v[3];

		if (!inside)
			vertices[clippedVertices++] = cVertex;
	}

	switch (clippedVertices)
	{
	// All 3 vertices outside view frustum, don't draw this triangle
	case 3:
		break;

	// Two vertices outside view frusum, this means we can both just move them to the edge
	case 2:
	{
		// The vertex that is still in the frustum
		uint8_t sharedVertex = 3 - (vertices[0] + vertices[1]);

		// The vertices that are outside the frusum
		uint8_t others[] =
		{
			(sharedVertex + 1) % 3,
			(sharedVertex + 2) % 3
		};

		for (uint8_t cVertex = 0; cVertex < 2; ++cVertex)
		{
			uint8_t other = others[cVertex];

			Vector4 onEdge = vtp[other].screenPosition;
			Vector4 toShared = vtp[sharedVertex].screenPosition - onEdge;

			// Clamp the coordinate to the edge of the view frustum
			onEdge[0] = Util::Clamp(onEdge[0], -onEdge[3], onEdge[3]);
			onEdge[1] = Util::Clamp(onEdge[1], -onEdge[3], onEdge[3]);
			onEdge[2] = Util::Clamp(onEdge[2], 0.0f, onEdge[3]);

			float d = cml::dot(onEdge, toShared);


			// TODO: Interpolate other vertex attributes to match onEdge position
		}

		break;
	}

	// One vertex outside view frustum, this means we have to create two triangles
	case 1:
	{
		break;
	}

	// All vertices inside view frustum, draw this triangla as-is
	case 0:
	{
		DispatchTriangle(vtp);
		break;
	}

	}
}

void SoftwareRenderer::DispatchTriangle(const SoftwareShader::VertexToPixel* vtp)
{
	SoftwareShader* shader = static_cast<SoftwareShader*>(currentMaterial->shader);

	TriangleData data;

	for (uint8_t cVertex = 0; cVertex < 3; ++cVertex)
	{
		// Copy vertex data to our triangle struct
		SoftwareShader::VertexToPixel& vertexData = (data.vertexToPixel[cVertex] = vtp[cVertex]);

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
	case DRAW_LINE: SortTriangle(&data.vertexToPixel[0], 0);
	case DRAW_FILL: SortTriangle(&data.vertexToPixel[0], 1);
	}

	Triangle2D* sst = &data.screenSpaceTriangle;

	// Screen-space triangle for bounds checking
	*sst = Triangle2D(Vector2(data.vertexToPixel[0].screenPosition[0], data.vertexToPixel[0].screenPosition[1]),
					  Vector2(data.vertexToPixel[1].screenPosition[0], data.vertexToPixel[1].screenPosition[1]),
					  Vector2(data.vertexToPixel[2].screenPosition[0], data.vertexToPixel[2].screenPosition[1]));

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

void SoftwareRenderer::DrawTilesST()
{
	for (uint32_t tileY = 0; tileY < verticalTiles; ++tileY)
	{
		for (uint32_t tileX = 0; tileX < horizontalTiles; ++tileX)
		{
			DrawTile(tileX, tileY);
		}
	}
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
	Point2 minTile(std::max(tileX * TILE_WIDTH, 0u), 
				   std::max(tileY * TILE_HEIGHT, 0u));

	Point2 maxTile(std::min(minTile[0] + TILE_WIDTH - 1, (int32_t)frameBuffer->width - 1),
				   std::min(minTile[1] + TILE_HEIGHT - 1, (int32_t)frameBuffer->height - 1));

	// Draw all triangles in the tile
	while (!bin.empty())
	{
		const TriangleData& data = bin.back(); 

		const Triangle2D& sst = data.screenSpaceTriangle;
		const SoftwareShader::VertexToPixel* a = &data.vertexToPixel[0];
		const SoftwareShader::VertexToPixel* b = &data.vertexToPixel[1];
		const SoftwareShader::VertexToPixel* c = &data.vertexToPixel[2];

		Point2 points[] =
		{
			Point2(sst[0][0], sst[0][1]), Point2(sst[1][0], sst[1][1]),
			Point2(sst[0][0], sst[0][1]), Point2(sst[2][0], sst[2][1]),
			Point2(sst[1][0], sst[1][1]), Point2(sst[2][0], sst[2][1]),
		};


		//
		// Bresenham's line algorithm
		//
		
		for (uint32_t lineIdx = 0; lineIdx < 3; ++lineIdx)
		{
			uint32_t b = lineIdx * 2;

			Point2& from = points[b + 0];
			Point2& to = points[b + 1];

			Point2 minLine = minTile;
			Point2 maxLine = maxTile;

			Point2 delta = to - from;
			int32_t sx = Util::Sign(delta[0]);
			int32_t sy = Util::Sign(delta[1]);

			// Create a transformation matrix that converts the line so that it points in the (+x, +)y quadrant and such that dx > dy
			// This is neccesary to not have to check all cases during the rasterization loop
			IntMatrix22 mtx;
			int32_t* m = mtx.data();

			// Check if the slope is larger than 1.0
			if (delta[1] * sy > delta[0] * sx)
			{
				// Find out of the sign of dx and dy are the same
				// This decides of our matrix needs to be 0 1 or 0 -1
				//										  1 0   -1  0
				// By checking this we don't change the quadrant the line is in after this transformation
				// Therefore we can use the original non-transformed coordinates for the next two checks
				int32_t ds = 1 - (sx == sy) * 2;
				m[0] = 0;
				m[1] = ds;
				m[2] = ds;
				m[3] = 0;
			}
			else
			{
				m[0] = 1;
				m[1] = 0;
				m[2] = 0;
				m[3] = 1;
			}

			// Make sure from.x is smaller than to.x
			// Otherwise, mirror x axis
			if (from[0] > to[0])
			{
				
				m[0] = -m[0];
				m[2] = -m[2];
			}

			// Make sure from.y is smaller than to.y
			// Otherwise, mirror y axis
			if (from[1] > to[1])
			{
				m[1] = -m[1];
				m[3] = -m[3];
			}

			// Only if m01 and m10 have a different sign, the inverse of mtx is not the same as mtx but -mtx
			IntMatrix22 invMtx = mtx * (1 - ((m[1] - m[2]) != 0) * 2);

			// Convert all points we use to the desired space
			from = from * mtx;
			to = to * mtx;
			minLine = minLine * mtx;
			maxLine = maxLine * mtx;
			delta = delta * mtx;

			// Check if the line is completely outside the tile we are rendering
			if (from[0] > maxLine[0] || from[1] > maxLine[1] || to[0] < minLine[0] || to[1] < minLine[1])
				continue;
			
			float error = 0.0f;
			float deltaError = abs((float)delta[1] / delta[0]);

			// Limit the line segment to within the tile
			Point2 p = from;

			Point2 toLine = minLine - p;
			if (toLine[0] > 0)
			{
				p[0] = minLine[0];
				p[1] += toLine[0] * deltaError;
			}

			
			for (; p[0] <= to[0]; ++p[0])
			{
				// Convert the point back to the screen space and plot it
				Point2 pixel = p * invMtx;
				frameBuffer->SetPixel(pixel[0], pixel[1], Color::WHITE);

				error += deltaError;

				if (error >= 0.5f)
				{
					++p[1];
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
