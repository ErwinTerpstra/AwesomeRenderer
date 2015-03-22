#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	// TODO: Make this a generic buffer (void*) and move the "byte" functionality to a ByteBufer subclass
	// Also split the color functionality to a ColorBuffer class
	class Buffer
	{

	public:
		enum Encoding
		{
			RGB24, RGBA32, BGR24, FLOAT32
		};

		uint32_t width, height, size;
		uint8_t stride, bpp;

		Encoding encoding;

		uchar* data;

	public:

		Buffer();
		virtual ~Buffer();

		virtual void Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding);
		virtual uint8_t GetStrideForDepth(uint8_t bitDepth);
		virtual void Destroy();
		
		__inline void Clear() { memset(data, 0, size); }
		void Clear(const Color& color);
		
		float GetPixel(uint32_t x, uint32_t y) const;
		void GetPixel(uint32_t x, uint32_t y, Color& color) const;

		void SetPixel(uint32_t x, uint32_t y, const Color& color);
		void SetPixel(uint32_t x, uint32_t y, const uchar* buffer);
		void SetPixel(uint32_t x, uint32_t y, float f);
		
		__inline uchar* GetBase(uint32_t x, uint32_t y) const
		{ 
			return data + (y * width + x) * stride;
		}

		static uint8_t GetEncodingDepth(Encoding encoding);

		static void EncodeColor(const Color& color, Encoding encoding, uchar* buffer);
		static void DecodeColor(const uchar* buffer, Encoding encoding, Color& color);

	};

}

#endif