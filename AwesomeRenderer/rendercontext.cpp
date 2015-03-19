#include "awesomerenderer.h"

#include "rendercontext.h"

using namespace AwesomeRenderer;

RenderContext::RenderContext() : camera(NULL), renderTarget(NULL), clearFlags(RenderTarget::BUFFER_ALL)
{

}
