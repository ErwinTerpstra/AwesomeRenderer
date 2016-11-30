#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "awesomerenderer.h"
#include "buffer.h"

namespace AwesomeRenderer
{
	class BufferAllocator;

	class Texture : public Buffer, public ExtensionProvider<Texture>
	{
	public:

		enum Extensions
		{
			TEXTURE_GL
		};

	public:
		Texture(BufferAllocator* allocator);

	};

}

#endif