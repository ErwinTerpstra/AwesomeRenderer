#include "stdafx.h"
#include "awesomerenderer.h"

#include "util_gl.h"

#include "texture_gl.h"
#include "texture.h"

using namespace AwesomeRenderer;

TextureGL::TextureGL(Texture& texture) : Extension(texture), id(0)
{

}

TextureGL::~TextureGL()
{
	if (id != 0)
		glDeleteTextures(1, &id);
}

void TextureGL::Load()
{
	GL_CHECK_ERROR(glGenTextures(1, &id));

	Bind();

	GL_CHECK_ERROR(glEnable(GL_TEXTURE_2D));

	GL_CHECK_ERROR(
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	);

	GL_CHECK_ERROR(
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	);
	
	GLenum interalFormat;
	GLenum dataFormat;
	GLint alignment;
	if (provider.bpp >= 32)
	{
		interalFormat = GL_RGBA8;
		dataFormat = GL_BGRA;
		alignment = 4;
	}
	else
	{
		interalFormat = GL_RGB8;
		dataFormat = GL_BGR;
		alignment = 1;
	}


	// Setup storage
	GL_CHECK_ERROR(glTexStorage2D(GL_TEXTURE_2D, provider.GetMipmapLevels(), interalFormat, provider.width, provider.height));

	// Upload source image
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	//GL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, interalFormat, provider.width, provider.height, 0, dataFormat, GL_UNSIGNED_BYTE, provider.data));
	GL_CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, provider.width, provider.height, dataFormat, GL_UNSIGNED_BYTE, provider.data));

	// Generate mipmaps
	GL_CHECK_ERROR(glGenerateMipmap(GL_TEXTURE_2D));

	Unbind();
}

void TextureGL::Bind()
{
	GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, id));
}

void TextureGL::Unbind()
{
	GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
}