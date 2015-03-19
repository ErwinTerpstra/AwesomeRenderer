#include "awesomerenderer.h"

#include "window.h"

#include "inputmanager.h"
#include "gdibuffer.h"

#include <assert.h>

using namespace AwesomeRenderer;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window(HINSTANCE instance, const char* className, bool createClass) : instance(instance), className(className)
{
	if (createClass)
		RegisterClass();
}

Window::~Window()
{
	
}

void Window::RegisterClass() const
{
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = instance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = className;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);
}

void Window::Create(const char* title, int width, int height)
{
	handle = CreateWindowEx(WS_EX_CLIENTEDGE, className, title, WS_OVERLAPPEDWINDOW,
			 			    CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, instance, NULL);
	assert(handle != NULL);

    SetWindowLongPtrW(handle, GWLP_USERDATA, (long)this);
	
	this->width = width;
	this->height = height;
	
	closed = false;
}

void Window::Show(int command) const
{
	ShowWindow(handle, command);
	UpdateWindow(handle);
}

void Window::ProcessMessages() const
{
	MSG msg;

	while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK Window::MessageCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CLOSE:
			DestroyWindow(hWnd);
			closed = true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		case WM_KEYDOWN:
			InputManager::Instance().KeyDown(wParam);
			break;

		case WM_KEYUP:
			InputManager::Instance().KeyUp(wParam);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

void Window::DrawBuffer(const GdiBuffer& buffer) const
{
	HDC windowDC = GetDC(handle);
	HDC bufferDC = CreateCompatibleDC(windowDC);
	HGDIOBJ oldObj = SelectObject(bufferDC, buffer.bitmap);
			
	BitBlt(windowDC, 0, 0, buffer.width, buffer.height, bufferDC, 0, 0, SRCCOPY);
			
	SelectObject(bufferDC, oldObj);
	DeleteDC(bufferDC);
	ReleaseDC(handle, windowDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    long userData = GetWindowLongW(hWnd, GWLP_USERDATA);
	Window* window = reinterpret_cast<Window*>(userData);

    return window->MessageCallback(hWnd, msg, wParam, lParam);
}