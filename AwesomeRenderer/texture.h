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
		const uint32_t DEFAULT_MIPMAP_LEVELS = 4;

		enum Extensions
		{
			TEXTURE_GL
		};
		
	private:
		uint32_t mipmapLevels;

	public:
		Texture(BufferAllocator* allocator, ColorSpace colorSpace);
		
		uint32_t GetMipmapLevels() const { return mipmapLevels; }

	};

}

#endif