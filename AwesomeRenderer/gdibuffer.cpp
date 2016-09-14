#include "stdafx.h"
#include "awesomerenderer.h"
#include "gdibuffer.h"

using namespace AwesomeRenderer;

GdiBuffer::GdiBuffer(HWND windowHandle) : windowHandle(windowHandle), Buffer()
{

}

GdiBuffer::~GdiBuffer()
{

}

void GdiBuffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding)
{
	// Only BGR24 encoding is supporte for the GDI buffer
	assert(encoding == BGR24);

	Buffer::Allocate(preferredWidth, preferredHeight, encoding);

	bitmapInfo.bmiColors[0].rgbRed		= 255;
	bitmapInfo.bmiColors[0].rgbGreen	= 255;
	bitmapInfo.bmiColors[0].rgbBlue		= 255;

	BITMAPINFOHEADER& header = bitmapInfo.bmiHeader;
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = this->bpp;
	header.biCompression = BI_RGB;
	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biSizeImage = size;
	header.biClrImportant = 0;
	header.biClrUsed = 0;
	
	HDC windowDC = GetDC(windowHandle);

	void* bufferBase = NULL;
	bitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_RGB_COLORS, &bufferBase, NULL, 0);

	data = static_cast<uchar*>(bufferBase);

	ReleaseDC(windowHandle, windowDC);
}

void GdiBuffer::Destroy()
{
	DeleteObject(bitmap);
}