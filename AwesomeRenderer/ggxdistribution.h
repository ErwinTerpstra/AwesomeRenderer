#ifndef _GGX_DISTRIBUTION_H_
#define _GGX_DISTRIBUTION_H_

#include "microfacetdistribution.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class GGXDistribution : public MicrofacetDistribution
		{
		public:

			virtual float Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const PbrMaterial& material) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const PbrMaterial& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const PbrMaterial& material);
		};
	}
}

#endif