#include <Windows.h>

#include "awesomerenderer.h"
#include "gdibuffer.h"

using namespace AwesomeRenderer;

GdiBuffer::GdiBuffer(HWND windowHandle) : windowHandle(windowHandle), Buffer()
{

}

GdiBuffer::~GdiBuffer()
{

}

void GdiBuffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, uint32_t stride)
{
	SetDimensions(preferredWidth, preferredHeight, stride);

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