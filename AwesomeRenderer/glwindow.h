#ifndef _WINDOW_OGL_H_
#define _WINDOW_OGL_H_

namespace AwesomeRenderer
{
	class GLWindow : public Extension<Window>
	{

	public:
		HDC deviceContext;

		HGLRC renderContext;

	public:
		GLWindow(Window& window);
		~GLWindow();

		bool Setup();
		void Draw();
	};

}

#endif