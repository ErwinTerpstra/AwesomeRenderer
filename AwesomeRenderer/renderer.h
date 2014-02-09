#ifndef _RENDERER_H_
#define _RENDERER_H_


namespace AwesomeRenderer
{

	class Renderer
	{
		
	public:
		enum DrawMode
		{
			DRAW_WIREFRAME,
			DRAW_SOLID
		};

		enum CullMode
		{
			CULL_NONE = 0,
			CULL_FRONT = -1,
			CULL_BACK = 1,
		};

		RenderContext* renderContext;

		CullMode cullMode;

	protected:
		Shader shader;

	public:

		virtual void Render() = 0;

	protected:
		Renderer();

		static void SortTriangle(Shader::VertexToPixel** a, Shader::VertexToPixel** b, Shader::VertexToPixel** c);

		template <typename T>
		static void Swap(T** a, T** b);


	};

}

#endif