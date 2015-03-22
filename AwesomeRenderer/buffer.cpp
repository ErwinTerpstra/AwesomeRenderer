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

void Buffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding)
{
	this->encoding = encoding;
	this->bpp = GetEncodingDepth(encoding);
	this->stride = GetStrideForDepth(bpp);
	this->width = preferredWidth;
	this->height = preferredHeight;
	this->size = width * height * stride;
}

uint8_t Buffer::GetStrideForDepth(uint8_t bitDepth)
{
	return bitDepth / 8;
}

void Buffer::Destroy()
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
	assert(bpp == sizeof(float) * 8);

	return *reinterpret_cast<float*>(GetBase(x, y));
}

void Buffer::GetPixel(uint32_t x, uint32_t y, Color& color) const
{
	uchar* pixelBase = GetBase(x, y);
	DecodeColor(pixelBase, encoding, color);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, float f)
{
	assert(bpp == sizeof(float) * 8);

	memcpy(GetBase(x, y), reinterpret_cast<uchar*>(&f), sizeof(float));
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const Color& color)
{
	uchar* pixelBase = GetBase(x, y);
	EncodeColor(color, encoding, pixelBase);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const uchar* buffer)
{
	uchar* pixelBase = GetBase(x, y);
	
	// Copy data from the input to the buffer
	memcpy(pixelBase, buffer, bpp / 8);
}


uint8_t Buffer::GetEncodingDepth(Encoding encoding)
{
	switch (encoding)
	{
	case RGB24:			return 24;
	case RGBA32:		return 32;
	case BGR24:			return 24;
	case FLOAT32:		return 32;
	}

	assert(false && "Encoding not supported.");
}

void Buffer::EncodeColor(const Color& color, Encoding encoding, uchar* buffer)
{
	switch (encoding)
	{
	case RGB24:
		buffer[0] = color[0] * 255.0f;
		buffer[1] = color[1] * 255.0f;
		buffer[2] = color[2] * 255.0f;
		break;

	case RGBA32:
		buffer[0] = color[0] * 255.0f;
		buffer[1] = color[1] * 255.0f;
		buffer[2] = color[2] * 255.0f;
		break;

	case BGR24:
		buffer[0] = color[2] * 255.0f;
		buffer[1] = color[1] * 255.0f;
		buffer[2] = color[0] * 255.0f;
		break;

	default:
		assert(false && "Encoding does not support color writing.");
		break;
	}

}

void Buffer::DecodeColor(const uchar* buffer, Encoding encoding, Color& color)
{
	switch (encoding)
	{
	case RGB24:
		color[0] = buffer[0] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[2] / 255.0f;
		color[3] = 1.0f;
		break;

	case RGBA32:
		color[0] = buffer[0] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[2] / 255.0f;
		color[3] = buffer[3] / 255.0f;
		break;

	case BGR24:
		color[0] = buffer[2] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[0] / 255.0f;
		color[3] = 1.0f;
		break;

	default:
		assert(false && "Encoding does not support color reading.");
		break;
	}
}