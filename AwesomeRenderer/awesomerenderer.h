#ifndef _AWESOME_RENDERER_H_
#define _AWESOME_RENDERER_H_

#define PI			3.14159265358979323846
#define TWO_PI		6.28318530717958647693
#define FOUR_PI		12.56637061435917295385
#define HALF_PI		1.57079632679489661923
#define FRTHRD_PI	4.18879020478639098462

#define INV_PI		1.0 / PI
#define INV_TWO_PI	1.0 / TWO_PI

// STD library
//#include <algorithm>
//#include <limits>
#include <memory>
#include <map>
#include <stack>
#include <queue>
#include <deque>
#include <math.h>
#include <thread>
#include <mutex>
#include <condition_variable>

// Windows libraries
#define NOMINMAX
#include <Windows.h>

#undef near
#undef far

#include "debug.h"

// OpenGL
#include <GL/glew.h>
#include <GL/wglew.h>

// Data types
#include "extension.h"
#include "extensionprovider.h"
#include "typedefs.h"
#include "color.h"

#include "util.h"

#endif