#ifndef _PHONG_SHADER_H_
#define _PHONG_SHADER_H_

#include "awesomerenderer.h"
#include "softwareshader.h"

namespace AwesomeRenderer
{

	class PhongShader : public SoftwareShader
	{

	public:
		
	public:
		PhongShader();
		
		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const;

	};

}

#endif