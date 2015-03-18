#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "awesomerenderer.h"
#include "rendercontext.h"

namespace AwesomeRenderer
{
	class Window;

	class Renderer
	{
		
	public:
		enum DrawMode
		{
			DRAW_LINE,
			DRAW_FILL,

			DRAW_DEFAULT = DRAW_FILL
		};

		enum CullMode
		{
			CULL_NONE = 0,
			CULL_FRONT = -1,
			CULL_BACK = 1,

			CULL_DEFAULT = CULL_BACK
		};

		CullMode cullMode = CULL_DEFAULT;

		DrawMode drawMode = DRAW_DEFAULT;
		
	protected:
		const RenderContext* renderContext;

	public:

		virtual void Initialize() = 0;
		virtual void Render() = 0;
		virtual void Present(Window& window) = 0;
		virtual void Cleanup() = 0;

		virtual void SetRenderContext(const RenderContext* renderContext);

	protected:
		Renderer();

	};

}

#endif