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

		CullMode cullMode;
		
	protected:
		const RenderContext* renderContext;

	public:

		virtual void Initialize() = 0;
		virtual void Render() = 0;

		virtual void SetRenderContext(const RenderContext* renderContext);

	protected:
		Renderer();

	};

}

#endif