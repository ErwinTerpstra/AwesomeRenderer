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
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			SetPixel(x, y, color);
		}
	}
}

float Buffer::GetPixel(int x, int y) const
{
	unsigned long max = (1 << bpp) - 1;
	unsigned long value = 0;
	
	uchar* pixelBase = GetBase(x, y);
	
	for (int i = 0; i < stride; ++i)
		value |= pixelBase[i] << (8 * i);

	return ((float) value) / max;
}

void Buffer::GetPixel(int x, int y, Color& sample) const
{
	uchar* base = GetBase(x, y);

	for (int i = 0; i < stride; ++i)
		sample[i] = *(base + i) / 255.0f;
}

void Buffer::SetPixel(int x, int y, float f)
{
	// Scales a float in 0 .. 1 range to the max value stored per pixel
	f = cml::clamp(f, 0.0f, 1.0f);
	unsigned long max = (1 << bpp) - 1;
	unsigned long value = (unsigned long) (f * max);
	
	uchar* pixelBase = GetBase(x, y);
	memcpy(pixelBase, reinterpret_cast<uchar*>(&value), stride);
}

void Buffer::SetPixel(int x, int y, const Color& color)
{
	uchar* pixelBase = GetBase(x, y);

	// Copy as much elements from color as fit per pixel
	for (int i = 0; i < stride; ++i)
		*(pixelBase + i) = (uchar) (color[i] * 255);
}

void Buffer::SetPixel(int x, int y, uchar value)
{
	// Set a single value for all channels per pixel
	memset(GetBase(x, y), value, stride);
}

void Buffer::SetPixel(int x, int y, const uchar* buffer)
{
	uchar* pixelBase = GetBase(x, y);
	
	// Copy as much elements from buffer as fit per pixel
	memcpy(pixelBase, buffer, stride);
}