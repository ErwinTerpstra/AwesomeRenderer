#ifndef _RENDER_TARGET_H_
#define _RENDER_TARGET_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Texture;

	class RenderTarget : public ExtensionProvider<RenderTarget>
	{

	public:
		enum BufferType
		{
			BUFFER_NONE = 0,
			BUFFER_COLOR = 1,
			BUFFER_DEPTH = 2,

			BUFFER_ALL = BUFFER_COLOR | BUFFER_DEPTH
		};

		enum Extensions
		{
			RENDER_TARGET_GL
		};


		BufferType buffers;
		
		Texture *frameBuffer, *depthBuffer;

	public:
		RenderTarget();
		
		void SetupBuffers(Texture* frameBuffer, Texture* depthBuffer);
		void Clear(const Color& color, BufferType buffers = BUFFER_ALL);

	};

}

#endif