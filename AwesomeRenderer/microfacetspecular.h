#ifndef _MICROFACET_SPECULAR_H_
#define _MICROFACET_SPECULAR_H_

#include "bxdf.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class MicrofacetSpecular : public BxDF
		{

		public:
			MicrofacetSpecular();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;

		private:
			Vector3 SpecularCookTorrance(const Vector3& v, const Vector3& n, const Vector3& l, float roughness) const;

			float RoughnessToShininess(float a) const;

			float DistributionBlinn(const Vector3& n, const Vector3& h, float e) const;
			float DistributionGGX(const Vector3& n, const Vector3& h, float alpha) const;

			float GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h) const;
			float GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h) const;
			float GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a) const;
			float GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a) const;

			float G1Schlick(const Vector3& v, const Vector3& n, float a) const;
			float G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a) const;

		};
	}
}

#endif