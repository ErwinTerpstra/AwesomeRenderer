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
		static const uint32_t DEFAULT_MIPMAP_LEVELS = 5;

		enum Extensions
		{
			TEXTURE_GL
		};
		
	private:
		uint32_t mipmapLevels;

		Buffer** mipChain;

	public:
		Texture(BufferAllocator* allocator, ColorSpace colorSpace);
		~Texture();

		void GenerateMipMaps();
		Buffer* GetMipLevel(uint32_t mipLevel);
		
		uint32_t GetMipmapLevels() const { return mipmapLevels; }
		bool HasMipmaps() const { return mipmapLevels > 0 && mipChain != NULL; }

	};

}

#endif