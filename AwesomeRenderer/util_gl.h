#ifndef _UTIL_GL_H_
#define _UTIL_GL_H_

#include "debug.h"

#define GL_NO_CALL

#define GL_CHECK_ERROR(CALL) \
{ \
	CALL; \
	do \
	{ \
		GLenum err = glGetError(); \
		if (err) \
		{ \
			printf("[OpenGL]: %s; at %s:%u: %s (0x%04x)\n", #CALL, __FILE__, __LINE__, \
			gluErrorString(err), err); \
			Debug::Break(); \
		} \
	} while (0); \
} 

#endif