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
		};

		struct VertexToPixel
		{
			Vector4 screenPosition;
			Vector4 worldPosition;
			Vector3 normal;
			Vector2 uv;
			Color color;
		};

		struct PixelInfo
		{
			Color color;
		};

		Matrix44 modelMtx, viewMtx, projMtx;
		Matrix44 screenMtx;

		const Material* material;

	public:
		Shader();

		void Prepare() { screenMtx = modelMtx * viewMtx * projMtx; }

		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const = 0;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const = 0;

	};

}

#endif