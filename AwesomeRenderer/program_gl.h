#ifndef _PROGRAM_GL_H_
#define _PROGRAM_GL_H_

#include "awesomerenderer.h"
#include "shader.h"
#include "shader_gl.h"

namespace AwesomeRenderer
{
	class TextureGL;

	class ProgramGL : public Shader
	{

	private:
		std::map<std::string, GLint> uniformLocations;
		std::map<std::string, GLint> attribLocations;

	public:
		GLuint handle;

	public:
		ProgramGL();
		~ProgramGL();

		void Prepare();

		void Attach(ShaderGL* shader);
		void Link();
		void Bind();

		void BindTexture(TextureGL* texture, std::string uniformName, GLenum slot);

		GLint GetUniformLocation(std::string name);
		GLint GetAttribLocation(std::string name);

		void SetAttribLocation(std::string name, GLuint location);
	};
}

#endif