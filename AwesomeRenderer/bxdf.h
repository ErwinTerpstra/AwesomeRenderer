#ifndef _BXDF_H_
#define _BXDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Material;
	struct RaycastHit;

	namespace RayTracing
	{

		class BxDF
		{
			

		public:

			BxDF();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material) const = 0;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const = 0;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const = 0;
			
			void SphericalToCartesian(float phi, float theta, Vector3& v) const;
			void TransformSampleVector(const Vector3& n, const Vector3& in, Vector3& out) const;

			template<typename T>
			static T FresnelSchlick(float cosT, T F0)
			{
				return F0 + (1.0f - F0) * pow(1 - cosT, 5);
			};

			static bool CalculateHalfVector(const Vector3& wo, const Vector3& wi, Vector3& h);
		};
	}
}

#endif