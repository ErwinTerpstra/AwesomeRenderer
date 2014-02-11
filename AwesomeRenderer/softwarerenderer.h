#ifndef _SOFTWARE_RENDERER_H_
#define _SOFTWARE_RENDERER_H_

namespace AwesomeRenderer
{

	class SoftwareRenderer : public Renderer
	{

	public:

	private:
		const Material* currentMaterial;

	public:
		SoftwareRenderer();
		~SoftwareRenderer();

		void Render();

		void BeginDraw(const Matrix44& model, const Material& material, DrawMode drawMode = DRAW_SOLID);
		void EndDraw();
		
		void DrawModel(const Model& model, Transformation& trans);

	private:
		void DrawTriangle(const Shader::VertexInfo* vertexBuffer);

	};
	
}

#endif