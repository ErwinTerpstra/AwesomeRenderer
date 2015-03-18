#ifndef _WINDOW_GL_H_
#define _WINDOW_GL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Window;

	class WindowGL : public Extension<Window>
	{

	public:
		HDC deviceContext;

		HGLRC renderContext;

	public:
		WindowGL(Window& window);
		~WindowGL();

		bool Setup();
		void Draw();
	};

}

#endif