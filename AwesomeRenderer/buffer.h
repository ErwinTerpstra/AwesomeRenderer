#ifndef _BUFFER_H_
#define _BUFFER_H_

namespace AwesomeRenderer
{

	class Buffer
	{

	public:
		uint32_t width, height, size;
		uint8_t stride, bpp;

		// TODO: Make this a generic buffer (void*) and move the "byte" functionality to a ByteBufer subclass
		uchar* data;

	public:

		Buffer();
		virtual ~Buffer();

		virtual void Allocate(uint32_t preferredWidth, uint32_t preferredHeight, uint32_t stride) = 0;
		virtual void Destroy() = 0;
		
		__inline void Clear() { memset(data, 0, size); }
		void Clear(const Color& color);
		
		float GetPixel(uint32_t x, uint32_t y) const;
		void GetPixel(uint32_t x, uint32_t y, Color& sample) const;

		void SetPixel(uint32_t x, uint32_t y, const Color& color);
		void SetPixel(uint32_t x, uint32_t y, const uchar* buffer);
		void SetPixel(uint32_t x, uint32_t y, uchar value);
		void SetPixel(uint32_t x, uint32_t y, float f);
		
		__inline uchar* GetBase(uint32_t x, uint32_t y) const
		{ 
			return data + (y * width + x) * (bpp / 8);
		}

	protected:

		void SetDimensions(uint32_t width, uint32_t height, uint32_t stride)
		{
			this->width = width;
			this->height = height;
			this->stride = stride;
			this->bpp = stride * 8;
			this->size = width * height * stride;
		}

	};

}

#endif