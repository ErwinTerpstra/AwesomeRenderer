#include <memory.h>

#include "awesomerenderer.h"
#include "buffer.h"

using namespace AwesomeRenderer;


Buffer::Buffer() : data(NULL)
{

}

Buffer::~Buffer()
{
	
}

void Buffer::Clear(const Color& color)
{
	for (uint8_t y = 0; y < height; ++y)
		for (uint8_t x = 0; x < width; ++x)
			SetPixel(x, y, color);
}

float Buffer::GetPixel(uint32_t x, uint32_t y) const
{
	assert(stride == 4);

	return *reinterpret_cast<float*>(GetBase(x, y));
}

void Buffer::GetPixel(uint32_t x, uint32_t y, Color& sample) const
{
	uchar* base = GetBase(x, y);

	for (uint8_t i = 0; i < stride; ++i)
		sample[i] = *(base + i) / 255.0f;
}

void Buffer::SetPixel(uint32_t x, uint32_t y, float f)
{
	memcpy(GetBase(x, y), reinterpret_cast<uchar*>(&f), stride);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const Color& color)
{
	uchar* pixelBase = GetBase(x, y);

	// Copy as much elements from color as fit per pixel
	for (uint8_t i = 0; i < stride; ++i)
		*(pixelBase + i) = (uchar) (color[i] * 255);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, uchar value)
{
	// Set a single value for all channels per pixel
	memset(GetBase(x, y), value, stride);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const uchar* buffer)
{
	uchar* pixelBase = GetBase(x, y);
	
	// Copy as much elements from buffer as fit per pixel
	memcpy(pixelBase, buffer, stride);
}