#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

namespace AwesomeRenderer
{

	class WindowGL;
	
	class RendererGL : public Renderer
	{

	public:

		WindowGL& window;

	private:

		const Material* currentMaterial;

		ProgramGL defaultShader;
		ShaderGL defaultVertex;
		ShaderGL defaultFragment;

	public:

		RendererGL(WindowGL& window);

		void Initialize();
		void Render();

	private:
		void BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode = DRAW_SOLID);
		void EndDraw();

		void DrawModel(const Model& model, const Transformation& trans);


	};

}

#endif