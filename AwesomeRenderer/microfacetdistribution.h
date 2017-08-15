#ifndef _MICROFACET_DISTRIBUTION_H_
#define _MICROFACET_DISTRIBUTION_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class PbrMaterial;

	namespace RayTracing
	{
		class MicrofacetDistribution
		{

		public:

			virtual float Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const PbrMaterial& material) const = 0;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const PbrMaterial& material, Vector3& wi) const = 0;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const PbrMaterial& material) = 0;
		};
	}
}

#endif;