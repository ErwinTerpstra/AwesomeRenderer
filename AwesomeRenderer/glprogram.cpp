#include "awesomerenderer.h"

using namespace AwesomeRenderer;

GLProgram::GLProgram()
{
	handle = glCreateProgram();
}

GLProgram::~GLProgram()
{
	

}

void GLProgram::Attach(GLShader* shader)
{
	glAttachShader(handle, shader->handle);
}

void GLProgram::Link()
{
	glLinkProgram(handle);
}

void GLProgram::Bind()
{
	glUseProgram(handle);
}

GLint GLProgram::GetUniformLocation(std::string name)
{
	std::map<std::string, GLint>::iterator it = uniformLocations.find(name);
	
	if (it == uniformLocations.end())
	{
		GLint location = glGetUniformLocation(handle, name.c_str());
		uniformLocations[name] = location;

		return location;
	}

	return it->second;
}