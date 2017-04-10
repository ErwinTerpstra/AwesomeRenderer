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

		GLuint handle;

	public:
		ProgramGL();
		~ProgramGL();

		void Prepare();

		void Attach(ShaderGL* shader);
		void Link();
		void Bind() const;

		void BindTexture(TextureGL* texture, std::string uniformName, GLenum slot);
		
		void SetVector3(const std::string& uniformName, const Vector3& v);
		void SetVector4(const std::string& uniformName, const Vector4& v);
		void SetMatrix44(const std::string& uniformName, const Matrix44& m);

		GLint GetUniformLocation(const std::string& name);
		GLint GetAttribLocation(const std::string& name);

		void SetAttribLocation(const std::string& name, GLuint location);
	};
}

#endif