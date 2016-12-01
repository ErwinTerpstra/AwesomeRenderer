#ifndef _TEXTURE_GL_H_
#define _TEXTURE_GL_H_

#include "awesomerenderer.h"
#include "texture.h"

namespace AwesomeRenderer
{
	class TextureGL : public Extension<Texture, TextureGL>
	{

	public:

		GLuint id;

	public:

		TextureGL(Texture& texture);
		~TextureGL();

		void Load();
		void Bind();
		void Unbind();

		static uint32_t ExtensionID() { return Texture::TEXTURE_GL; }
	};
}

#endif