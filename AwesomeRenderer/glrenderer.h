#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

namespace AwesomeRenderer
{

	class GLWindow;

	class GLRenderer : public Renderer
	{

	public:

		GLWindow& window;

	private:

		const Material* currentMaterial;

		GLProgram defaultShader;
		GLShader defaultVertex;
		GLShader defaultFragment;

	public:

		GLRenderer(GLWindow& window);


		void Render();

	private:
		void BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode = DRAW_SOLID);
		void EndDraw();

		void DrawModel(const Model& model, const Transformation& trans);


	};

}

#endif