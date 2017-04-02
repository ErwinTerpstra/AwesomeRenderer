#include "stdafx.h"
#include "awesomerenderer.h"
#include "shader_gl.h"

using namespace AwesomeRenderer;

ShaderGL::ShaderGL(GLenum type)
{
	handle = glCreateShader(type);
}

ShaderGL::~ShaderGL()
{

}

bool ShaderGL::Compile(const char** source, int amount)
{
	glShaderSource(handle, amount, source, NULL);
	glCompileShader(handle);

	int status;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	if (!status) 
	{
		printf("[ShaderGL]: Error in compiling shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(handle, 10239, &length, log);
		
		printf("[ShaderGL]: Compiler log : \n%s\n", log);

		return FALSE;
	}

	return TRUE;
}