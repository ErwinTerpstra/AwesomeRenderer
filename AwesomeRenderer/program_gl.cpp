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

	if (texture != NULL)
		texture->Bind();
	else
		TextureGL::ClearBoundTexture();

	glUniform1i(GetUniformLocation(uniformName), slot - GL_TEXTURE0);

	GL_CHECK_ERROR("BindTexture");
}

void ProgramGL::SetVector3(const std::string& uniformName, const Vector3& v)
{
	glUniform3f(GetUniformLocation(uniformName), v[0], v[1], v[2]);
}

void ProgramGL::SetVector4(const std::string& uniformName, const Vector4& v)
{
	glUniform4f(GetUniformLocation(uniformName), v[0], v[1], v[2], v[3]);
}

void ProgramGL::SetMatrix44(const std::string& uniformName, const Matrix44& m)
{
	glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, m.data());	
}

GLint ProgramGL::GetUniformLocation(const std::string& name)
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

GLint ProgramGL::GetAttribLocation(const std::string& name)
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

void ProgramGL::SetAttribLocation(const std::string& name, GLuint location)
{
	glBindAttribLocation(handle, location, name.c_str());
	attribLocations[name] = location;
}