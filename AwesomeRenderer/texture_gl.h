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

		void Create();
		void Load();
		void Bind();

		static void ClearBoundTexture();

		static void GetEncodingParameters(Buffer::Encoding encoding, GLenum& internalFormat, GLenum& dataFormat, GLenum& dataType);

		static uint32_t ExtensionID() { return Texture::TEXTURE_GL; }
	};
}

#endif