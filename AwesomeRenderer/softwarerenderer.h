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

		template<typename T>
		class LockedVariable
		{
		private:
			T value;

			std::mutex mtx;

		public:
			LockedVariable(const T& initialValue) : value(initialValue)
			{

			}

			void Lock() { mtx.lock(); }
			void Unlock() { mtx.unlock();  }

			T& operator*() { return value; }
			T* operator->() { return &value; }
		};

		class Counter
		{
		private:
			std::mutex m;
			std::condition_variable_any signal;

			uint32_t count, maxCount;
		public:
			Counter();

			void Configure(uint32_t count, uint32_t maxCount);

			void Reset();
			void Decrement();
			void WaitZero();
		};

		class Semaphore
		{
		private:
			std::mutex m;
			std::condition_variable_any signal;

			uint32_t count;
			const uint32_t maxCount;
		public:
			Semaphore(uint32_t count, uint32_t maxCount);
			void Signal(uint32_t increment = 1);
			void Wait();
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