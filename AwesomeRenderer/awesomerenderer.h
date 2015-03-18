#ifndef _AWESOME_RENDERER_H_
#define _AWESOME_RENDERER_H_

#define PI         3.14159265358979323846
#define TWO_PI     6.28318530717958647693
#define FOUR_PI   12.56637061435917295385
#define HALF_PI    1.57079632679489661923
#define FRTHRD_PI  4.18879020478639098462

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
#include "extendee.h"
#include "typedefs.h"
#include "color.h"

/*
// Memory
#include "buffer.h"
#include "memorybuffer.h"

// Utility
#include "util.h"
#include "timer.h"
#include "transformation.h"
#include "ray.h"
#include "raycasthit.h"
#include "camera.h"

// Mult-threading
#include "threading.h"

// Primitives
#include "object.h"
#include "primitive.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "triangle.h"
#include "triangle3d.h"
#include "triangle2d.h"

// Input
#include "inputmanager.h"
#include "cameracontroller.h"

// Scene graph
#include "node.h"
#include "kdtree.h"
#include "octree.h"

// Rendering
#include "texture.h"
#include "sampler.h"
#include "material.h"

#include "mesh.h"
#include "model.h"

#include "textmesh.h"

#include "meshex.h"
#include "modelex.h"

// Renderer
#include "shader.h"
#include "softwareshader.h"
#include "unlitshader.h"
#include "phongshader.h"

#include "rendertarget.h"
#include "rendercontext.h"
#include "renderer.h"

#include "softwarerenderer.h"
#include "raytracer.h"

// Assets
#include "factory.h"
#include "filereader.h"
#include "texturefactory.h"
#include "objloader.h"

// Win32
#include "gdibuffer.h"
#include "window.h"

// OpenGL
#include "mesh_gl.h"
#include "texture_gl.h"
#include "window_gl.h"
#include "shader_gl.h"
#include "program_gl.h"
#include "renderer_gl.h"
*/
#endif