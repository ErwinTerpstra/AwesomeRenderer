#include "awesomerenderer.h"

#include "util_gl.h"

#include "texture_gl.h"
#include "texture.h"

using namespace AwesomeRenderer;

TextureGL::TextureGL(Texture& texture) : Extension(texture)
{

}

TextureGL::~TextureGL()
{
	glDeleteTextures(1, &id);
}

void TextureGL::Load()
{
	GL_CHECK_ERROR(glGenTextures(1, &id));

	Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	GLenum interalFormat;
	GLenum dataFormat;
	GLint alignment;
	if (provider.bpp >= 32)
	{
		interalFormat = GL_RGBA;
		dataFormat = GL_BGRA;
		alignment = 4;
	}
	else
	{
		interalFormat = GL_RGB;
		dataFormat = GL_BGR;
		alignment = 1;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glTexImage2D(GL_TEXTURE_2D, 0, interalFormat, provider.width, provider.height, 0, dataFormat, GL_UNSIGNED_BYTE, provider.data);
}

void TextureGL::Bind()
{
	glBindTexture(GL_TEXTURE_2D, id);
}