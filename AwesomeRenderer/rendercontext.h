#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

#include "awesomerenderer.h"
#include "rendertarget.h"

namespace AwesomeRenderer
{
	class Window;
	class Camera;
	class Node;
	class LightData;
	class Skybox;

	class RenderContext
	{

	public:
		RenderTarget::BufferType clearFlags;

		Camera* camera;
		RenderTarget* renderTarget;

		LightData* lightData;

		Skybox* skybox;

		std::vector<Node*> nodes;

	public:

		RenderContext();

	};
}


#endif