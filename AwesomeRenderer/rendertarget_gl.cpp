#include "stdafx.h"

#include "util_gl.h"

#include "rendertarget_gl.h"
#include "texture_gl.h"

using namespace AwesomeRenderer;

RenderTargetGL::RenderTargetGL(RenderTarget& renderTarget) : Extension(renderTarget), renderTargetID(0), depthBufferID(0)
{

}

RenderTargetGL::~RenderTargetGL()
{
	if (renderTargetID != 0)
		glDeleteFramebuffers(1, &renderTargetID);

	if (depthBufferID != 0)
		glDeleteRenderbuffers(1, &depthBufferID);
}

void RenderTargetGL::Load()
{
	GL_CHECK_ERROR(glGenFramebuffers(1, &renderTargetID));

	Bind();

	if (provider.frameBuffer != NULL)
	{
		frameBuffer = new TextureGL(*provider.frameBuffer);
		frameBuffer->Load();

		// Bind the frame buffer as COLOR_ATTACHMENT0
		GL_CHECK_ERROR(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBuffer->id, 0));

		// Set the list of draw buffers.
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		GL_CHECK_ERROR(glDrawBuffers(1, drawBuffers));
	}
	
	if (provider.depthBuffer != NULL)
	{
		// Setup a depth buffer
		GL_CHECK_ERROR(glGenRenderbuffers(1, &depthBufferID));
		GL_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID));
		GL_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, provider.depthBuffer->width, provider.depthBuffer->height));
		GL_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID));
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("[RenderTargetGL]: Error setting up GL render target!");
}

void RenderTargetGL::Bind()
{
	GL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, renderTargetID));
}

void RenderTargetGL::Unbind()
{
	GL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void RenderTargetGL::Read()
{
	if (provider.frameBuffer != NULL)
	{
		glReadBuffer((GLenum)GL_COLOR_ATTACHMENT0_EXT);
		GL_CHECK_ERROR(glReadPixels(0, 0, provider.frameBuffer->width, provider.frameBuffer->height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)provider.frameBuffer->data));
	}

	if (provider.depthBuffer != NULL)
		GL_CHECK_ERROR(glReadPixels(0, 0, provider.depthBuffer->width, provider.depthBuffer->height, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*)provider.depthBuffer->data));
}