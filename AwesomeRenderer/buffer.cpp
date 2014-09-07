#include <memory.h>

#include "awesomerenderer.h"

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
	unsigned long max = (1 << bpp) - 1;
	unsigned long value = 0;
	
	uchar* pixelBase = GetBase(x, y);
	
	for (uint8_t i = 0; i < stride; ++i)
		value |= pixelBase[i] << (8 * i);

	return ((float) value) / max;
}

void Buffer::GetPixel(uint32_t x, uint32_t y, Color& sample) const
{
	uchar* base = GetBase(x, y);

	for (uint8_t i = 0; i < stride; ++i)
		sample[i] = *(base + i) / 255.0f;
}

void Buffer::SetPixel(uint32_t x, uint32_t y, float f)
{
	// Scales a float in 0 .. 1 range to the max value stored per pixel
	f = cml::clamp(f, 0.0f, 1.0f);
	unsigned long max = (1 << bpp) - 1;
	unsigned long value = (unsigned long) (f * max);
	
	uchar* pixelBase = GetBase(x, y);
	memcpy(pixelBase, reinterpret_cast<uchar*>(&value), stride);
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