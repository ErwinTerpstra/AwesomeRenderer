#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "program_gl.h"
#include "shader_gl.h"

namespace AwesomeRenderer
{
	class Window;
	class WindowGL;
	class Model;
	class Material;
	class Transformation;
	class Mesh;

	class RendererGL : public Renderer
	{

	private:
		enum SortMode
		{
			BACK_TO_FRONT,
			FRONT_TO_BACK
		};

		struct RenderJob
		{
			const Mesh* mesh;
			const Material* material;
			const Transformation* trans;
		};

		struct RenderQueue
		{
			SortMode sortMode;
			std::deque<RenderJob> jobs;
		};

		class JobComparator
		{
		public:

		private:
			Vector3 cameraPosition;

			SortMode sortMode;

		public:
			JobComparator(const Vector3& cameraPosition, SortMode sortMode);

			bool operator()(const RenderJob& a, const RenderJob& b);
		};

		RenderQueue opaque, transparent;

		ProgramGL defaultShader;
		ShaderGL defaultVertex;
		ShaderGL defaultFragment;

	public:

		RendererGL();

		void Initialize();
		void Render();
		void Present(Window& window);
		void Cleanup();

	private:
		void PreRender();
		void PostRender();

		void EnqueueModel(const Model& model, const Transformation& trans);

		void SortRenderQueue(RenderQueue& queue);
		void DrawRenderQueue(RenderQueue& queue);
		void DrawJob(const RenderJob& job);

		void BeginDraw(const Matrix44& model, const Material& material);
		void EndDraw();

		GLenum GetCullMode() const
		{
			switch (cullMode)
			{
			case CULL_BACK:	return GL_BACK;
			case CULL_FRONT: return GL_FRONT;
			default: return GL_NONE;
			}
		}

	};

}

#endif