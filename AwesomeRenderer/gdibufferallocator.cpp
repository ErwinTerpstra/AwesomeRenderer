#include "stdafx.h"
#include "awesomerenderer.h"
#include "gdibufferallocator.h"

using namespace AwesomeRenderer;

GDIBufferAllocator::GDIBufferAllocator(HWND windowHandle) : BufferAllocator(), windowHandle(windowHandle)
{

}

GDIBufferAllocator::~GDIBufferAllocator()
{

}

uchar* GDIBufferAllocator::Allocate(const Buffer& buffer)
{
	// Only BGR24 encoding is supporte for the GDI buffer
	assert(buffer.encoding == Buffer::BGR24);

	bitmapInfo.bmiColors[0].rgbRed		= 255;
	bitmapInfo.bmiColors[0].rgbGreen	= 255;
	bitmapInfo.bmiColors[0].rgbBlue		= 255;

	BITMAPINFOHEADER& header = bitmapInfo.bmiHeader;
	header.biWidth = buffer.width;
	header.biHeight = buffer.height;
	header.biPlanes = 1;
	header.biBitCount = buffer.bpp;
	header.biCompression = BI_RGB;
	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biSizeImage = buffer.size;
	header.biClrImportant = 0;
	header.biClrUsed = 0;
	
	HDC windowDC = GetDC(windowHandle);

	void* bufferBase = NULL;
	bitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_RGB_COLORS, &bufferBase, NULL, 0);

	uchar* data = static_cast<uchar*>(bufferBase);

	ReleaseDC(windowHandle, windowDC);

	return data;
}

void GDIBufferAllocator::Destroy()
{
	DeleteObject(bitmap);
}