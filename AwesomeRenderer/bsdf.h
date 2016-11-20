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
			Vector3 SpecularTradeoff(const Vector3& diffuseRadiance, const Vector3& specularRadiance, const Vector3& n, const Vector3& v, const Material& material) const;

		private:
			Vector3 GetF0(const Material& material) const;

		public:
			template<typename T>
			static T FresnelSchlick(float cosT, T F0)
			{
				return F0 + (1.0f - F0) * pow(1 - cosT, 5);
			};
		};
	}
}

#endif