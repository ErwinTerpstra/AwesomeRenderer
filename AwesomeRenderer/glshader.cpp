#include "awesomerenderer.h"

using namespace AwesomeRenderer;

GLShader::GLShader(GLenum type)
{
	handle = glCreateShader(type);
}

GLShader::~GLShader()
{

}

void GLShader::Compile(const char** source, int amount)
{
	glShaderSource(handle, amount, source, NULL);
	glCompileShader(handle);

	int status;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	if (!status) 
	{
		printf("[GLShader]: Error in compiling shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(handle, 10239, &length, log);
		
		printf("[GLShader]: Compiler log : \n%s\n", log);
	}
}