#ifndef _SHADER_H_
#define _SHADER_H_

namespace AwesomeRenderer
{

	class Shader
	{

	public:
		struct VertexInfo
		{
			Vector3 position;
			Vector3 normal;
			Vector2 uv;
			Color color;

			VertexInfo() : position(), normal(0.0f, 1.0f, 0.0f), uv(0.0f, 0.0f), color(Color::WHITE)
			{

			}
		};

		struct VertexToPixel
		{
			Vector4 screenPosition;
			Vector4 worldPosition;
			Vector3 normal;
			Vector2 uv;
			Color color;

			VertexToPixel() : screenPosition(), worldPosition(), normal(), uv(0.0f, 0.0f), color(Color::WHITE)
			{

			}
		};

		struct PixelInfo
		{
			Color color;
		};

		Matrix44 modelMtx, viewMtx, projMtx;
		
		Matrix44 screenMtx;
		Vector4 viewPosition;

		const Material* material;

	public:
		Shader();

		void Prepare();

		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const = 0;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const = 0;

	};

}

#endif