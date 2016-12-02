#include "stdafx.h"
#include "awesomerenderer.h"

#include "texture.h"
#include "bufferallocator.h"

using namespace AwesomeRenderer;

Texture::Texture(BufferAllocator* allocator) : Buffer(allocator), mipmapLevels(DEFAULT_MIPMAP_LEVELS)
{

}
