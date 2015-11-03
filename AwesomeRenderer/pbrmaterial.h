#ifndef _PBR_MATERIAL_H_
#define _PBR_MATERIAL_H_

#include "awesomerenderer.h"
#include "material.h"

namespace AwesomeRenderer
{

	class PbrMaterial : public Material
	{
	public:

		float roughness;

		float metallic;

		Color albedo;

		Color specular;

	public:
		PbrMaterial();

	};

}


#endif