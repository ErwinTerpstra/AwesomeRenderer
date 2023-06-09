#ifndef _MICROFACET_SPECULAR_H_
#define _MICROFACET_SPECULAR_H_

#include "bxdf.h"

namespace AwesomeRenderer
{
	class RenderContext;
	class MicrofacetMaterial;

	namespace RayTracing
	{
		class MicrofacetDistribution;

		class MicrofacetSpecular : public BxDF
		{
		private:
			MicrofacetDistribution* normalDistribution;

		public:
			MicrofacetSpecular();
			~MicrofacetSpecular();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;

		private:
			Vector3 SpecularCookTorrance(const Vector3& wo, const Vector3& normal, const Vector3& wi, const Vector3& F0, const MicrofacetMaterial& material) const;
			
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