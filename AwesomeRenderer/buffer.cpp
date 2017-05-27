#include "stdafx.h"
#include <memory.h>

#include "awesomerenderer.h"
#include "buffer.h"
#include "bufferallocator.h"

#include "inputmanager.h"

// TODO: move to color buffer cpp file
#include "sampler.h"
#include "texture.h"

using namespace AwesomeRenderer;

Buffer::Buffer(BufferAllocator* allocator, ColorSpace colorSpace) : allocator(allocator), colorSpace(colorSpace), data(NULL)
{
	
}

Buffer::~Buffer()
{
	Destroy();
}

const BufferAllocator& Buffer::GetAllocator() const
{
	return *allocator;
}

void Buffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding)
{
	AllocateAligned(preferredWidth, preferredHeight, 1, encoding);
}

void Buffer::AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding)
{
	this->encoding = encoding;
	this->alignment = alignment;
	
	this->bpp = GetEncodingDepth(encoding);
	this->pixelStride = this->bpp / 8;

	this->width = preferredWidth;
	this->height = preferredHeight;

	this->stride = CalculateStride(preferredWidth, bpp, alignment);
	this->size = height * stride;

	data = allocator->Allocate(*this);
}

uint32_t Buffer::CalculateStride(uint32_t width, uint8_t bitDepth, uint8_t alignment)
{
	uint32_t stride = width * (bitDepth / 8);
	return (uint32_t) (ceil(stride / (float)alignment) * alignment);
}

void Buffer::Destroy()
{
	if (data != NULL)
	{
		allocator->Destroy();

		data = NULL;
	}
}

void Buffer::Clear(const Color& color)
{
	for (uint8_t y = 0; y < height; ++y)
		for (uint8_t x = 0; x < width; ++x)
			SetPixel(x, y, color);
}

void Buffer::Blit(const Buffer& src)
{
	bool tonemap = IsHDR(src.encoding) && !IsHDR(encoding) && !InputManager::Instance().GetKey('T');
	Color color;

	for (uint32_t y = 0; y < std::min(height, src.height); ++y)
	{
		for (uint32_t x = 0; x < std::min(width, src.width); ++x)
		{
			src.GetPixel(x, y, color);

			if (tonemap)
				Tonemap(color, color);

			if (colorSpace == GAMMA && src.colorSpace == LINEAR)
				AdjustGamma(color, 1.0f / DEFAULT_GAMMA);
			else if (colorSpace == LINEAR && src.colorSpace == GAMMA)
				AdjustGamma(color, DEFAULT_GAMMA);

			SetPixel(x, y, color);
		}
	}
}

void Buffer::Blit(const Sampler& sampler)
{
	bool tonemap = IsHDR(sampler.texture->encoding) && !IsHDR(encoding) && !InputManager::Instance().GetKey('T');
	Color color;

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			Vector2 uv(x / (float)width, y / (float)height);
			sampler.Sample(uv, color);

			if (tonemap)
				Tonemap(color, color);

			if (colorSpace == GAMMA)
				AdjustGamma(color, 1.0f / DEFAULT_GAMMA);

			SetPixel(x, y, color);
		}
	}
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

void Buffer::GetPixel(uint32_t x, uint32_t y, Color& color, ColorSpace colorSpace) const
{
	GetPixel(x, y, color);

	if (colorSpace == GAMMA && this->colorSpace == LINEAR)
		AdjustGamma(color, 1.0f / DEFAULT_GAMMA);
	else if (colorSpace == LINEAR && this->colorSpace == GAMMA)
		AdjustGamma(color, DEFAULT_GAMMA);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, float f)
{
	assert(encoding == FLOAT32);

	memcpy(GetBase(x, y), reinterpret_cast<uchar*>(&f), sizeof(float));
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const Color& color)
{
	uchar* pixelBase = GetBase(x, y);
	EncodeColor(color, encoding, pixelBase);
}

void Buffer::SetPixel(uint32_t x, uint32_t y, const Color& color, ColorSpace colorSpace)
{
	Color correctedColor = color;

	if (colorSpace == GAMMA && this->colorSpace == LINEAR)
		AdjustGamma(correctedColor, DEFAULT_GAMMA);
	else if (colorSpace == LINEAR && this->colorSpace == GAMMA)
		AdjustGamma(correctedColor, 1.0f / DEFAULT_GAMMA);

	SetPixel(x, y, correctedColor);
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
	case BGRA32:		return 32;
	case FLOAT32:		return 32;
	case FLOAT96:		return 96;
	case FLOAT128:		return 128;
	}

	assert(false && "Encoding not supported.");

	return 0;
}

void Buffer::EncodeColor(const Color& color, Encoding encoding, uchar* buffer)
{
	switch (encoding)
	{
	case RGB24:
		buffer[0] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		break;

	case RGBA32:
		buffer[0] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[3] = (uchar)(Util::Clamp01(color[3]) * 255.0f);
		break;

	case BGR24:
		buffer[0] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		break;

	case BGRA32:
		buffer[0] = (uchar)(Util::Clamp01(color[2]) * 255.0f);
		buffer[1] = (uchar)(Util::Clamp01(color[1]) * 255.0f);
		buffer[2] = (uchar)(Util::Clamp01(color[0]) * 255.0f);
		buffer[3] = (uchar)(Util::Clamp01(color[3]) * 255.0f);
		break;

	case FLOAT96:
	{
		float* floatBuffer = reinterpret_cast<float*>(buffer);
		floatBuffer[0] = color[0];
		floatBuffer[1] = color[1];
		floatBuffer[2] = color[2];
		break;
	}

	case FLOAT128:
	{
		float* floatBuffer = reinterpret_cast<float*>(buffer);
		floatBuffer[0] = color[0];
		floatBuffer[1] = color[1];
		floatBuffer[2] = color[2];
		floatBuffer[3] = color[3];
		break;
	}

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

	case BGRA32:
		color[0] = buffer[2] / 255.0f;
		color[1] = buffer[1] / 255.0f;
		color[2] = buffer[0] / 255.0f;
		color[3] = buffer[3] / 255.0f;
		break;

	case FLOAT96:
	{
		const float* floatBuffer = reinterpret_cast<const float*>(buffer);
		color[0] = floatBuffer[0];
		color[1] = floatBuffer[1];
		color[2] = floatBuffer[2];
		break;
	}

	case FLOAT128:
	{
		const float* floatBuffer = reinterpret_cast<const float*>(buffer);
		color[0] = floatBuffer[0];
		color[1] = floatBuffer[1];
		color[2] = floatBuffer[2];
		color[3] = floatBuffer[3];
		break;
	}

	default:
		assert(false && "Encoding does not support color reading.");
		break;
	}

}

bool Buffer::IsHDR(Encoding encoding)
{
	return encoding == FLOAT96 || encoding == FLOAT128;
}

void Buffer::Tonemap(const Color& hdr, Color& ldr)
{
	ldr[0] = Tonemap(hdr[0]);
	ldr[1] = Tonemap(hdr[1]);
	ldr[2] = Tonemap(hdr[2]);
	
	ldr[3] = 1.0f;
}

float Buffer::Tonemap(float x)
{
	// Very basic reinhard tonemapping
	//return x / (x + 1);

	// Approximation of Aces filming tonemapping curve:
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
	
	return Util::Clamp01((x*(a*x + b)) / (x*(c*x + d) + e));
}

void Buffer::AdjustGamma(Color& color, float gamma)
{
	color[0] = pow(color[0], gamma);
	color[1] = pow(color[1], gamma);
	color[2] = pow(color[2], gamma);
}