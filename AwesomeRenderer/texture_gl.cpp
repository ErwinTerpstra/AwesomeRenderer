
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

void TextureGL::Create()
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

	GLenum internalFormat;
	GLenum dataFormat;
	GLenum dataType;
	GetEncodingParameters(provider.encoding, internalFormat, dataFormat, dataType);

	// Setup storage
	GL_CHECK_ERROR(glTexStorage2D(GL_TEXTURE_2D, provider.GetMipmapLevels(), internalFormat, provider.width, provider.height));
	
	ClearBoundTexture();
}

void TextureGL::Load()
{	
	Bind();
		
	GLenum internalFormat;
	GLenum dataFormat;
	GLenum dataType;
	GetEncodingParameters(provider.encoding, internalFormat, dataFormat, dataType);

	// Upload source image
	GLint alignment = provider.alignment;
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

	GL_CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, provider.width, provider.height, dataFormat, dataType, provider.data));

	// Generate mipmaps
	GL_CHECK_ERROR(glGenerateMipmap(GL_TEXTURE_2D));

	ClearBoundTexture();
}

void TextureGL::Bind()
{
	GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, id));
}

void TextureGL::ClearBoundTexture()
{
	GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
}

void TextureGL::GetEncodingParameters(Buffer::Encoding encoding, GLenum& internalFormat, GLenum& dataFormat, GLenum& dataType)
{
	switch (encoding)
	{
		case Buffer::BGR24:
			internalFormat = GL_RGB8;
			dataFormat = GL_BGR;
			dataType = GL_UNSIGNED_BYTE;
			break;

		case Buffer::BGRA32:
			internalFormat = GL_RGBA8;
			dataFormat = GL_BGRA;
			dataType = GL_UNSIGNED_BYTE;
			break;

		case Buffer::RGB24:
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			dataType = GL_UNSIGNED_BYTE;
			break;

		case Buffer::RGBA32:
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			dataType = GL_UNSIGNED_BYTE;
			break;

		case Buffer::FLOAT128:
			internalFormat = GL_RGBA_FLOAT32_ATI;
			dataFormat = GL_RGBA;
			dataType = GL_FLOAT;
			break;

		default:
			printf("[TextureGL]: Invalid source texture format\n");
			assert(false);
			break;
	}
}