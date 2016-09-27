#ifndef _GDI_BUFFER_H_
#define _GDI_BUFFER_H_

#include "awesomerenderer.h"
#include "buffer.h"

namespace AwesomeRenderer
{

	class GdiBuffer : public Buffer
	{
		
	public:
		HBITMAP bitmap;

	private:
		
		HWND windowHandle;
		BITMAPINFO bitmapInfo;

	public:
		GdiBuffer(HWND windowHandle);
		~GdiBuffer();
		
		virtual void AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding);
		virtual void Destroy();

	};

}


#endif