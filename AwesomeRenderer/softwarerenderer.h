#ifndef _SOFTWARE_RENDERER_H_
#define _SOFTWARE_RENDERER_H_

namespace AwesomeRenderer
{

	class SoftwareRenderer : public Renderer
	{

	public:

	private:
		struct RenderJob
		{
			const Mesh* mesh;
			const Material* material;
			const Transformation* trans;
		};

		const Material* currentMaterial;

		std::deque<RenderJob> renderQueue;

	public:
		SoftwareRenderer();
		~SoftwareRenderer();

		void Render();
		void DrawModel(const Model& model, const Transformation& trans);

	private:

		void DrawJob(const RenderJob& job);

		void BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode = DRAW_SOLID);
		void DrawMesh(const Mesh& mesh);
		void EndDraw();

		void DrawTriangle(const SoftwareShader::VertexInfo* vertexBuffer);

		static void Blend(const Color& src, const Color& dst, Color& out);
		static void SortTriangle(SoftwareShader::VertexToPixel** a, SoftwareShader::VertexToPixel** b, SoftwareShader::VertexToPixel** c);

		template <typename T>
		static void Swap(T** a, T** b);


	};
	
}

#endif