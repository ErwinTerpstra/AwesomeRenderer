#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Shader::Shader()
{

}

void Shader::ProcessVertex(const VertexInfo& in, VertexToPixel& out) const
{
	Vector4 position(in.position, 1.0f);

	out.worldPosition = position * modelMtx;
	out.screenPosition = position * screenMtx;
	out.normal = in.normal;
	out.color = in.color;

	out.uv = in.uv;
}

void Shader::ProcessPixel(const VertexToPixel& in, PixelInfo& out) const
{
	material->texture->Sample(in.uv, out.color);

	//out.color = Color(fabs(in.normal[0]), fabs(in.normal[1]), fabs(in.normal[2]));
}