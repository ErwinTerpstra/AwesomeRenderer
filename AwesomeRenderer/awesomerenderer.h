#ifndef _AWESOME_RENDERER_H_
#define _AWESOME_RENDERER_H_

#include <memory>
#include <map>

#include "typedefs.h"

// Data types
#include "color.h"

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

// Shapes
#include "object.h"
#include "shape.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "triangle.h"
#include "triangle3d.h"
#include "triangle2d.h"

// Input
#include "inputmanager.h"
#include "cameracontroller.h"

// Rendering
#include "texture.h"
#include "material.h"
#include "mesh.h"
#include "meshex.h"
#include "model.h"
#include "modelex.h"

// Scene graph
#include "node.h"
#include "kdtree.h"
#include "octree.h"

// Renderer
#include "shader.h"
#include "softwareshader.h"
#include "phongshader.h"

#include "rendertarget.h"
#include "rendercontext.h"
#include "renderer.h"
#include "softwarerenderer.h"
#include "raytracer.h"

// Assets
#include "filereader.h"
#include "texturefactory.h"
#include "objloader.h"

// Windows specific
#ifdef _WIN32
	#define NOMINMAX
	#include <Windows.h>

	#undef near
	#undef far
	
	#include "gdibuffer.h"
	#include "window.h"
#endif

#endif