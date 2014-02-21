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

		const RenderContext* renderContext;

		CullMode cullMode;
		
	public:

		virtual void Render() = 0;

	protected:
		Renderer();

	};

}

#endif