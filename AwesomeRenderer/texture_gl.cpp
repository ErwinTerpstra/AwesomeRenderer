#include "awesomerenderer.h"

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
	glGenTextures(1, &id);

	Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	GLenum interalFormat;
	GLenum dataFormat;
	if (base.bpp >= 32)
	{
		interalFormat = GL_RGBA;
		dataFormat = GL_BGRA;
	}
	else
	{
		interalFormat = GL_RGB;
		dataFormat = GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, interalFormat, base.width, base.height, 0, dataFormat, GL_UNSIGNED_BYTE, base.data);
}

void TextureGL::Bind()
{
	glBindTexture(GL_TEXTURE_2D, id);
}