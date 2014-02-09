#ifndef _RENDER_TARGET_H_
#define _RENDER_TARGET_H_

namespace AwesomeRenderer
{

	class RenderTarget
	{

	public:
		enum BufferType
		{
			BUFFER_NONE = 0,
			BUFFER_COLOR = 1,
			BUFFER_DEPTH = 2,

			BUFFER_ALL = BUFFER_COLOR | BUFFER_DEPTH
		};

		BufferType buffers;
		
		Buffer *frameBuffer, *depthBuffer;

	public:
		RenderTarget();
		
		void SetupBuffers(Buffer* frameBuffer, Buffer* depthBuffer);
		void Clear(const Color& color, BufferType buffers = BUFFER_ALL);

	};

}

#endif