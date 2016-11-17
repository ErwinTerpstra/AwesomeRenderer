#ifndef _BXDF_H_
#define _BXDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Material;

	namespace RayTracing
	{

		class BxDF
		{
			

		public:

			BxDF();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const = 0;
			virtual void GenerateSampleVector(const Vector2& r, const Material& material, float& phi, float& theta, float& pdf) const = 0;
		};
	}
}

#endif