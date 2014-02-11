#ifndef _DIFFUSE_H_
#define _DIFFUSE_H_


namespace AwesomeRenderer
{

	class DiffuseShader : public Shader
	{

	public:
		DiffuseShader();
		
		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const;

	};

}

#endif