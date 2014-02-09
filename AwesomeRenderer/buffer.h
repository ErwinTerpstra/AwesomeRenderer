#ifndef _BUFFER_H_
#define _BUFFER_H_

namespace AwesomeRenderer
{

	class Buffer
	{

	public:
		int width, height, stride, bpp, size;

		// TODO: Make this a generic buffer (void*) and move the "byte" functionality to a ByteBufer subclass
		uchar* data;

	public:

		Buffer();
		virtual ~Buffer();

		virtual void Allocate(int preferredWidth, int preferredHeight, int stride) = 0;
		virtual void Destroy() = 0;
		
		__inline void Clear() { memset(data, 0, size); }
		void Clear(const Color& color);
		
		float GetPixel(int x, int y) const;
		void GetPixel(int x, int y, Color& sample) const;

		void SetPixel(int x, int y, const Color& color);
		void SetPixel(int x, int y, const uchar* buffer);
		void SetPixel(int x, int y, uchar value);
		void SetPixel(int x, int y, float f);
		
		__inline uchar* GetBase(int x, int y) const
		{ 
			return data + (y * width + x) * (bpp / 8);
		}

	protected:

		void SetDimensions(int width, int height, int stride)
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