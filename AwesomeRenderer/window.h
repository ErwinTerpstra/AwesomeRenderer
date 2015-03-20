#ifndef _WINDOW_H_
#define _WINDOW_H_

namespace AwesomeRenderer
{
	class GdiBuffer;

	class Window : public Extendee<Window>
	{

	public:
		HWND handle;
		
		bool closed;

		int width;
		int height;

	private:
		HINSTANCE instance;

		const char* className;

	public:

		Window(HINSTANCE instance, const char* className, bool createClass = true);
		~Window();
		
		void Create(const char* title, int width, int height);
		void Show(int command) const;
		void SetClientSize(int width, int height);
		
		void ProcessMessages() const;
		
		void DrawBuffer(const GdiBuffer& buffer) const;

		LRESULT CALLBACK MessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		
		void RegisterClass() const;



	};

}


#endif