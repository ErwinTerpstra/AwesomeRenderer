#include "unlitshader.h"
#include "material.h"
#include "sampler.h"

using namespace AwesomeRenderer;

UnlitShader::UnlitShader() : SoftwareShader()
{

}

void UnlitShader::ProcessVertex(const VertexInfo& in, VertexToPixel& out) const
{
	Vector4 position(in.position, 1.0f);

	out.worldPosition = position * modelMtx;
	out.screenPosition = position * screenMtx;

	Vector4 normal(in.normal, 0.0f);
	normal = normal * modelMtx;
	out.normal = normal.subvector(3).normalize();

	out.color = in.color;

	out.uv = in.uv;
}

void UnlitShader::ProcessPixel(const VertexToPixel& in, PixelInfo& out) const
{
	Color diffuse = material->diffuseColor;

	// Sample diffuse map if it is present
	if (material->diffuseMap)
		diffuse *= material->diffuseMap->Sample(in.uv);
	
	out.color = diffuse;
}