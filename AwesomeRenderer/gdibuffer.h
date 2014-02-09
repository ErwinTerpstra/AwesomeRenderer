#ifndef _GDI_BUFFER_H_
#define _GDI_BUFFER_H_

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
		
		virtual void Allocate(int preferredWidth, int preferredHeight, int stride);
		virtual void Destroy();

	};

}


#endif