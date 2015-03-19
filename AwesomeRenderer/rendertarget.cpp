#include "awesomerenderer.h"

#include "rendertarget.h"
#include "buffer.h"

using namespace AwesomeRenderer;

RenderTarget::RenderTarget() : frameBuffer(NULL), depthBuffer(NULL)
{

}

void RenderTarget::SetupBuffers(Buffer* frameBuffer, Buffer* depthBuffer)
{
	this->frameBuffer = frameBuffer;
	this->depthBuffer = depthBuffer;

	printf("[RenderTarget]: Frame buffer %dx%d %dbpp\n", frameBuffer->width, frameBuffer->height, frameBuffer->bpp);
	printf("[RenderTarget]: Depth buffer %dx%d %dbpp\n", depthBuffer->width, depthBuffer->height, depthBuffer->bpp);
}

void RenderTarget::Clear(const Color& color, RenderTarget::BufferType buffers)
{
	if ((buffers & BUFFER_COLOR) > 0 && frameBuffer != NULL)
		frameBuffer->Clear();

	if ((buffers & BUFFER_DEPTH) > 0 && depthBuffer != NULL)
		depthBuffer->Clear();
}
