#ifndef _MICROFACET_DISTRIBUTION_H_
#define _MICROFACET_DISTRIBUTION_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class MicrofacetMaterial;

	namespace RayTracing
	{
		class MicrofacetDistribution
		{

		public:

			virtual float Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material) const = 0;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const MicrofacetMaterial& material, Vector3& wi) const = 0;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material) = 0;
		};
	}
}

#endif;