#ifndef _GL_PROGRAM_H_
#define _GL_PROGRAM_H_

namespace AwesomeRenderer
{
	class GLProgram;

	class GLProgram
	{

	private:
		std::map<std::string, GLint> uniformLocations;

	public:
		GLuint handle;

	public:
		GLProgram();
		~GLProgram();

		void Attach(GLShader* shader);
		void Link();
		void Bind();

		GLint GetUniformLocation(std::string name);
	};
}

#endif