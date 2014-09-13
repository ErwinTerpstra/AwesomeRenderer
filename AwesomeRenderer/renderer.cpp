#include "awesomerenderer.h"

using namespace AwesomeRenderer;


Renderer::Renderer() : cullMode(CULL_BACK)
{

}

void Renderer::SetRenderContext(const RenderContext* context)
{
	this->renderContext = context;
}