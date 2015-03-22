#include "awesomerenderer.h"

#include "rendercontext.h"
#include "lightdata.h"

using namespace AwesomeRenderer;

RenderContext::RenderContext() : camera(NULL), renderTarget(NULL), lightData(NULL), skybox(NULL), clearFlags(RenderTarget::BUFFER_ALL)
{

}
