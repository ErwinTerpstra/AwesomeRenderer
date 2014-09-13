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
			HANDLE workerSemaphore;
		};

		class WorkerThread
		{
		private:

			HANDLE handle;
			DWORD id;

			bool available, running;
			HANDLE readHandle, writeHandle;
			HANDLE workerSemaphore;

			uint32_t tileX, tileY;

		public:

			WorkerThread();

			void Start(WorkerData& data);
			void Stop();

			void DrawTile(uint32_t tileX, uint32_t tileY);

			void WaitUntilAvailable();
			void WaitForData();
			void SetAvailable();

			bool IsAvailable() const { return available; }
			bool IsRunning() const { return running; }

			uint32_t TileX() const { return tileX; }
			uint32_t TileY() const { return tileY; }
		};


		const Material* currentMaterial;

		std::deque<RenderJob> renderQueue;

		std::vector<std::vector<TriangleData> > tiles;
		WorkerThread workers[WORKER_AMOUNT];
		HANDLE availableWorkers;

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

		void DrawJob(const RenderJob& job);

		void DrawTiles();
		void DrawTile(uint32_t tileX, uint32_t tileY);

		void BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode = DRAW_SOLID);
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
		static void SortTriangle(SoftwareShader::VertexToPixel* vtp);
		static void SortTriangle(SoftwareShader::VertexToPixel** a, SoftwareShader::VertexToPixel** b, SoftwareShader::VertexToPixel** c);

		template <typename T>
		static void Swap(T& a, T& b);

		template <typename T>
		static void Swap(T** a, T** b);


	};
	
}

#endif