#ifndef _WINDOW_H_
#define _WINDOW_H_

namespace AwesomeRenderer
{

	class Window
	{

	public:
		HWND handle;
		
		bool closed;

	private:
		HINSTANCE instance;

		const char* className;

	public:

		Window(HINSTANCE instance, const char* className, bool createClass = true);
		~Window();
		
		void Create(const char* title, int width, int height);
		void Show(int command);
		
		void ProcessMessages();
		
		void DrawBuffer(const GdiBuffer& buffer);

		LRESULT CALLBACK MessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		
		void RegisterClass();



	};

}


#endif