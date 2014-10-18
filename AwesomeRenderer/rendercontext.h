#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

namespace AwesomeRenderer
{
	class Window;

	class RenderContext
	{

	public:

		Camera* camera;
		RenderTarget* renderTarget;
		Window* window;

		std::vector<Node*> nodes;

	public:

		RenderContext();

	};
}


#endif