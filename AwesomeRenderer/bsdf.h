#ifndef _BSDF_H_
#define _BSDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Material;

	namespace RayTracing
	{
		class BxDF;

		class BSDF
		{
		public:
			BxDF* diffuse;
			BxDF* specular;

		public:

			BSDF();
			BSDF(BxDF* diffuse, BxDF* specular);

			Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;
			Vector3 ConserveEnergy(const Vector3& diffuseRadiance, const Vector3& specularRadiance) const;
		};
	}
}

#endif