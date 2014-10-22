#ifndef _UNLIT_SHADER_H_
#define _UNLIT_SHADER_H_

namespace AwesomeRenderer
{

	class UnlitShader : public SoftwareShader
	{

	public:

	public:
		UnlitShader();

		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const;

	};

}

#endif