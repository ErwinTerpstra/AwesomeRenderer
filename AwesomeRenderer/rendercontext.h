#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

#include "awesomerenderer.h"
#include "rendertarget.h"

#include "kdtree.h"

namespace AwesomeRenderer
{
	class Window;
	class Camera;
	class Node;
	class LightData;
	class Skybox;
	class Renderable;

	class RenderContext
	{

	public:
		RenderTarget::BufferType clearFlags;

		Camera* camera;
		RenderTarget* renderTarget;

		LightData* lightData;

		Skybox* skybox;

		// TODO: move this to a Scene class?
		std::vector<Node*> nodes;
		KDTree<Renderable> tree;

	public:

		RenderContext();

		void Optimize();

		void Update();

	};
}


#endif