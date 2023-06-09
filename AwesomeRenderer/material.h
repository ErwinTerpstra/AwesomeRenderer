#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "awesomerenderer.h"

#include "bsdf.h"

namespace AwesomeRenderer
{
	class Shader;
	class Sampler;

	class Material : public ExtensionProvider<Material>
	{
	public:
		enum Extensions
		{
			MATERIAL_PHONG,
			MATERIAL_PBR
		};

		std::string name;

		Sampler* normalMap;

		bool translucent;

		float ior;

		Color emission;
		float emissionIntensity;

		// TODO: Move shader to SoftwareRenderer/OpenGL renderer specific material
		Shader* shader;

		// TODO: Move BSDF to Raytracer specific material
		RayTracing::BSDF* bsdf;

	public:
		Material();
		virtual ~Material() { };
	};

}


#endif