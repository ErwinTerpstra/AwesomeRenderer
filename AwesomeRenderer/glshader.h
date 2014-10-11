#ifndef _GL_SHADER_H_
#define _GL_SHADER_H_

namespace AwesomeRenderer
{
	class GLShader : Shader
	{

	public:
		GLuint handle;

	public:
		GLShader(GLenum type);
		~GLShader();

		void Compile(const char** source, int amount);

	};
}

#endif