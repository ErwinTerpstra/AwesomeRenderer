#ifndef _SHADER_GL_H_
#define _SHADER_GL_H_

namespace AwesomeRenderer
{
	class ShaderGL
	{

	public:
		GLuint handle;

	public:
		ShaderGL(GLenum type);
		~ShaderGL();
		
		void Compile(const char** source, int amount);

	};
}

#endif