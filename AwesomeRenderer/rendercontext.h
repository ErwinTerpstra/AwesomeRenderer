#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

namespace AwesomeRenderer
{
	class Window;

	class RenderContext
	{

	public:
		RenderTarget::BufferType clearFlags;

		Camera* camera;
		RenderTarget* renderTarget;

		std::vector<Node*> nodes;

	public:

		RenderContext();

	};
}


#endif