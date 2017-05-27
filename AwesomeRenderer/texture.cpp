#include "stdafx.h"
#include "awesomerenderer.h"

#include "texture.h"
#include "bufferallocator.h"
#include "memorybufferallocator.h"

using namespace AwesomeRenderer;

Texture::Texture(BufferAllocator* allocator, ColorSpace colorSpace) : Buffer(allocator, colorSpace), mipmapLevels(DEFAULT_MIPMAP_LEVELS), mipChain(NULL)
{

}

Texture::~Texture()
{
	if (mipChain != NULL)
	{
		for (uint32_t mipmapLevel = 0; mipmapLevel < mipmapLevels; ++mipmapLevel)
			delete mipChain[mipmapLevel];

		delete[] mipChain;
	}
}

void Texture::GenerateMipMaps()
{
	assert(data != NULL);

	Buffer* previousLevel = this;
	Color samples[4];

	mipChain = new Buffer*[mipmapLevels];

	for (uint32_t mipmapLevel = 1; mipmapLevel <= mipmapLevels; ++mipmapLevel)
	{
		Buffer* mipBuffer = new Buffer(new MemoryBufferAllocator(), colorSpace);
		mipBuffer->Allocate(width >> mipmapLevel, height >> mipmapLevel, encoding);

		for (uint32_t y = 0; y < mipBuffer->height; ++y)
		{
			for (uint32_t x = 0; x < mipBuffer->width; ++x)
			{
				// Sample the four pixels used for this level evenly from the previous buffer
				uint32_t baseX = x << 1;
				uint32_t baseY = y << 1;
				previousLevel->GetPixel(baseX, baseY, samples[0], Buffer::LINEAR);
				previousLevel->GetPixel(baseX + 1, baseY, samples[1], Buffer::LINEAR);
				previousLevel->GetPixel(baseX, baseY + 1, samples[2], Buffer::LINEAR);
				previousLevel->GetPixel(baseX + 1, baseY + 1, samples[3], Buffer::LINEAR);

				Color sample = (samples[0] + samples[1] + samples[2] + samples[3]) * 0.25f;
				mipBuffer->SetPixel(x, y, sample, Buffer::LINEAR);
			}
		}

		mipChain[mipmapLevel - 1] = mipBuffer;
		previousLevel = mipBuffer;
	}
}

Buffer* Texture::GetMipLevel(uint32_t mipLevel)
{
	if (mipLevel > 0)
	{
		assert(mipLevel <= mipmapLevels && HasMipmaps());
		return mipChain[mipLevel - 1];
	}

	return static_cast<Buffer*>(this);
}