#ifndef _TEXTURE_GL_H_
#define _TEXTURE_GL_H_

namespace AwesomeRenderer
{
	class TextureGL : public Extension<Texture>
	{

	public:

		GLuint id;

	public:

		TextureGL(Texture& texture);
		~TextureGL();

		void Load();
		void Bind();
	};
}

#endif