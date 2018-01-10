#ifndef _BLINN_DISTRIBUTION_H_
#define _BLINN_DISTRIBUTION_H_

#include "microfacetdistribution.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class BlinnDistribution : public MicrofacetDistribution
		{
		public:

			virtual float Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const MicrofacetMaterial& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const MicrofacetMaterial& material);

			static float RoughnessToShininess(float roughness)
			{
				// Brian karis' method
				return std::max((2.0f / powf(roughness, 3)) - 2.0f, 1e-7f);
			}

			static float ShininessToRoughness(float shininess)
			{
				// Brian karis' method
				return powf(2.0f / (shininess + 2.0f), 1.0f / 3.0f);
			}


		};
	}
}

#endif