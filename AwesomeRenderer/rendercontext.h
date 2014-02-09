#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

namespace AwesomeRenderer
{
	
	class RenderContext
	{

	public:

		Camera* camera;
		RenderTarget* renderTarget;

		std::vector<Node*> nodes;

	public:

		RenderContext();

	};
}


#endif