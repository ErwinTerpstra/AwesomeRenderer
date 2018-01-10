
#include "awesomerenderer.h"

#include "window_gl.h"
#include "window.h"

#include "util_gl.h"
#include "filereader.h"

#include "texture_gl.h"

using namespace AwesomeRenderer;

WindowGL::WindowGL(Window& window) : Extension(window)
{
	deviceContext = GetDC(window.handle);
}

WindowGL::~WindowGL()
{
	wglMakeCurrent(deviceContext, 0); // Remove the rendering context from our device context  
	wglDeleteContext(renderContext); // Delete our rendering context  
}

bool WindowGL::Setup()
{
	// Setup a pixel format descriptor to setup our window pixel format
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA; 
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE; 

	// Check if the choosen pixel format is supported
	int pixelFormatIdx = ChoosePixelFormat(deviceContext, &pfd);
	
	if (pixelFormatIdx == 0)
		return false;

	// Try to apply the choosen pixel format
	if (!SetPixelFormat(deviceContext, pixelFormatIdx, &pfd))
		return false; 
	
	// Create and activate a temporary context to create the final one
	HGLRC tempContext = wglCreateContext(deviceContext); 
	wglMakeCurrent(deviceContext, tempContext);
	
	// Initialize GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK)
		return false;

	// Create attribute array to use to setup our GL Context
	int attributes[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4, 
		WGL_CONTEXT_MINOR_VERSION_ARB, 0, 
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	// Check if the OpenGL 3.x context creation extension is available  
	// NOTE: wglCreateContextAttribsARB seems to give a context on which glEnableClientState fails. Fix this!
	if (false && wglewIsSupported("WGL_ARB_create_context") == 1) 
	{ 
		renderContext = wglCreateContextAttribsARB(deviceContext, 0, attributes);

		assert(renderContext != NULL);
		assert(GetLastError() == NO_ERROR);

		wglMakeCurrent(deviceContext, NULL);
		wglDeleteContext(tempContext);
		wglMakeCurrent(deviceContext, renderContext);
	}
	else 
	{
		// Fallback context if we can't get our desired version context
		renderContext = tempContext;
	}
		
	int glVersion[2] = { -1, -1 };
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

	printf("[WindowGL]: Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

	GL_CHECK_ERROR(glEnableClientState(GL_VERTEX_ARRAY));
	
	return true;
}

void WindowGL::Draw()
{
	SwapBuffers(deviceContext);
}