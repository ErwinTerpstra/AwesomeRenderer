#ifndef _SOFTWARE_RENDERER_H_
#define _SOFTWARE_RENDERER_H_

namespace AwesomeRenderer
{

	class SoftwareRenderer : public Renderer
	{

	public:
		static const int TILE_WIDTH = 32, TILE_HEIGHT = 32;

		static const int WORKER_AMOUNT = 8;

	private:
		struct RenderJob
		{
			const Mesh* mesh;
			const Material* material;
			const Transformation* trans;
		};

		struct TriangleData
		{
			Triangle2D screenSpaceTriangle;
			SoftwareShader::VertexToPixel vertexToPixel[3];

			TriangleData() : screenSpaceTriangle(Vector2(), Vector2(), Vector2())
			{

			}
		};

		class WorkerThread;
		struct WorkerData
		{
			WorkerThread* thread;
			SoftwareRenderer* renderer;
		};

		class WorkerThread
		{

		public:
			uint32_t tileX, tileY;

		private:

			HANDLE handle;
			DWORD id;

			bool available, running;

		public:

			WorkerThread();

			void Start(WorkerData& data);
			void Stop();

			bool IsRunning() const { return running; }
		};

		const Material* currentMaterial;

		std::deque<RenderJob> renderQueue;
		
		std::vector<std::vector<TriangleData> > tiles;
		Semaphore workerSignal, mainThreadSignal;
		
		LockedVariable<uint32_t> tileIdx;
		Counter tilesLeft;

		WorkerThread workers[WORKER_AMOUNT];

		uint32_t horizontalTiles, verticalTiles;

	public:
		SoftwareRenderer();
		~SoftwareRenderer();

		void Initialize();
		void Cleanup();

		void Render();
		void DrawModel(const Model& model, const Transformation& trans);

		void SetRenderContext(const RenderContext* context);

	private:

		void PreRender();
		void PostRender();

		void DrawJob(const RenderJob& job);

		void DrawTiles();
		void DrawTile(uint32_t tileX, uint32_t tileY);
		void DrawTileFill(uint32_t tileX, uint32_t tileY);
		void DrawTileLine(uint32_t tileX, uint32_t tileY);

		void BeginDraw(const Matrix44& model, const Material& material);
		void DrawMesh(const Mesh& mesh);
		void EndDraw();

		void DrawTriangle(const SoftwareShader::VertexInfo* vertexBuffer);

		DWORD StartWorker(WorkerThread* thread);

		static DWORD WINAPI HandleWorker(LPVOID args)
		{
			WorkerData* workerData = static_cast<WorkerData*>(args);
			return workerData->renderer->StartWorker(workerData->thread);
		}

		static void Blend(const Color& src, const Color& dst, Color& out);
		static void SortTriangle(SoftwareShader::VertexToPixel* vtp, uint32_t axis);
		static void SortTriangle(SoftwareShader::VertexToPixel** a, SoftwareShader::VertexToPixel** b, SoftwareShader::VertexToPixel** c, uint32_t axis);

	};
	
}

#endif