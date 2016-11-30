#ifndef _GDI_BUFFER_ALLOCATOR_H_
#define _GDI_BUFFER_ALLOCATOR_H_

#include "awesomerenderer.h"
#include "bufferallocator.h"

#include "buffer.h"

namespace AwesomeRenderer
{

	class GDIBufferAllocator : public BufferAllocator
	{
		
	public:
		HBITMAP bitmap;

	private:
		
		HWND windowHandle;
		BITMAPINFO bitmapInfo;

	public:
		GDIBufferAllocator(HWND windowHandle);
		~GDIBufferAllocator();
		
		virtual uchar* Allocate(const Buffer& buffer);
		virtual void Destroy();

	};

}


#endif