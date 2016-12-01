#include "stdafx.h"
#include "awesomerenderer.h"

#include "util_gl.h"
#include "program_gl.h"
#include "texture_gl.h"

using namespace AwesomeRenderer;

ProgramGL::ProgramGL()
{
	handle = glCreateProgram();
}

ProgramGL::~ProgramGL()
{
	

}

void ProgramGL::Prepare()
{
	Bind();
}

void ProgramGL::Attach(ShaderGL* shader)
{
	GL_CHECK_ERROR(glAttachShader(handle, shader->handle));
}

void ProgramGL::Link()
{
	GL_CHECK_ERROR(glLinkProgram(handle));
}

void ProgramGL::Bind()
{
	GL_CHECK_ERROR(glUseProgram(handle));
}

void ProgramGL::BindTexture(TextureGL* texture, std::string uniformName, GLenum slot)
{
	glActiveTexture(slot);

	texture->Bind();

	glUniform1i(GetUniformLocation(uniformName), slot - GL_TEXTURE0);

	GL_CHECK_ERROR("BindTexture");
}

GLint ProgramGL::GetUniformLocation(std::string name)
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

GLint ProgramGL::GetAttribLocation(std::string name)
{
	std::map<std::string, GLint>::iterator it = attribLocations.find(name);

	if (it == attribLocations.end())
	{
		GLint location = glGetAttribLocation(handle, name.c_str());
		attribLocations[name] = location;

		return location;
	}

	return it->second;
}

void ProgramGL::SetAttribLocation(std::string name, GLuint location)
{
	glBindAttribLocation(handle, location, name.c_str());
	attribLocations[name] = location;
}