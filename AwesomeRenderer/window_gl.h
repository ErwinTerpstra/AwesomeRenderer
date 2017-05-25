#ifndef _WINDOW_GL_H_
#define _WINDOW_GL_H_

#include "awesomerenderer.h"
#include "window.h"

namespace AwesomeRenderer
{
	class TextureGL;

	class WindowGL : public Extension<Window, WindowGL>
	{

	public:
		HDC deviceContext;

		HGLRC renderContext;
		
	public:
		WindowGL(Window& window);
		~WindowGL();

		bool Setup();
		void Draw();

		void DrawImage(TextureGL& texture);

		static uint32_t ExtensionID() { return Window::WINDOW_GL; }
	};

}

#endif