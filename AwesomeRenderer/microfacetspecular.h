#ifndef _MICROFACET_SPECULAR_H_
#define _MICROFACET_SPECULAR_H_

#include "bsdf.h"

namespace AwesomeRenderer
{
	namespace RayTracing
	{
		class MicrofacetSpecular : public BSDF
		{

		public:
			MicrofacetSpecular();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material);

		private:

			Vector3 DiffuseLambert(const Vector3& albedo);
			Vector3 SpecularCookTorrance(const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks);

			float RoughnessToShininess(float a);

			float DistributionBlinn(const Vector3& n, const Vector3& h, float e);
			float DistributionGGX(const Vector3& n, const Vector3& h, float alpha);

			float GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h);
			float GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h);
			float GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a);
			float GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a);

			float G1Schlick(const Vector3& v, const Vector3& n, float a);
			float G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a);

			Vector3 FresnelSchlick(float cosT, Vector3 F0);

		};
	}
}

#endif