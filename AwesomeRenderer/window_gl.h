#ifndef _WINDOW_GL_H_
#define _WINDOW_GL_H_

namespace AwesomeRenderer
{
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