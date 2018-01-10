
#include "awesomerenderer.h"

#include "renderer.h"

using namespace AwesomeRenderer;


Renderer::Renderer() : cullMode(CULL_BACK), renderContext(NULL)
{

}

void Renderer::SetRenderContext(const RenderContext* context)
{
	this->renderContext = context;
}