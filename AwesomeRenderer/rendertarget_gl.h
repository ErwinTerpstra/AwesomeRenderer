#ifndef _RENDER_TARGET_GL_H_
#define _RENDER_TARGET_GL_H_

#include "awesomerenderer.h"
#include "rendertarget.h"

namespace AwesomeRenderer
{
	class TextureGL;

	class RenderTargetGL : public Extension<RenderTarget, RenderTargetGL>
	{

	private:
		TextureGL* frameBuffer;

		GLuint renderTargetID;
		GLuint depthBufferID;

	public:
		RenderTargetGL(RenderTarget& renderTarget);
		~RenderTargetGL();

		void Load();
		void Bind();
		void Unbind();

		void Read();
		
		static uint32_t ExtensionID() { return RenderTarget::RENDER_TARGET_GL; }
	};
}

#endif