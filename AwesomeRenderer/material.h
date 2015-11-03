#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Shader;
	class Sampler;

	class Material
	{
	public:
		bool translucent;

		Shader* shader;

	public:
		Material();

	};


}


#endif