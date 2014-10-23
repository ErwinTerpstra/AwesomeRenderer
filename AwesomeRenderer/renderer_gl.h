#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

namespace AwesomeRenderer
{

	class WindowGL;
	
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


	};

}

#endif