#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "program_gl.h"
#include "shader_gl.h"

namespace AwesomeRenderer
{
	class Window;
	class WindowGL;
	class Model;
	class Material;
	class Transformation;

	class RendererGL : public Renderer
	{

	private:

		const Material* currentMaterial;

		ProgramGL defaultShader;
		ShaderGL defaultVertex;
		ShaderGL defaultFragment;

	public:

		RendererGL();

		void Initialize();
		void Render();
		void Present(Window& window);
		void Cleanup();

	private:
		void PreRender();
		void PostRender();

		void BeginDraw(const Matrix44& model, const Material& material);
		void EndDraw();

		void DrawModel(const Model& model, const Transformation& trans);

		GLenum GetCullMode() const
		{
			switch (cullMode)
			{
			case CULL_BACK:	return GL_BACK;
			case CULL_FRONT: return GL_FRONT;
			default: return GL_NONE;
			}
		}

	};

}

#endif