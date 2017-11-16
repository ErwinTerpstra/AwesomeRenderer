#ifndef _BSDF_H_
#define _BSDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class RenderContext;
	class Material;
	struct RaycastHit;

	namespace RayTracing
	{
		class BxDF;

		class BSDF
		{
		public:
			enum BxDFTypes
			{
				BXDF_DIFFUSE = 1,
				BXDF_SPECULAR = 2,

				BXDF_ALL = 255,
			};

			BxDF* diffuse;
			BxDF* specular;

		public:

			BSDF();
			BSDF(BxDF* diffuse, BxDF* specular);

			Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext, BxDFTypes typeMask = BXDF_ALL) const;

			void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const;
			float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;

		};
	}
}

#endif