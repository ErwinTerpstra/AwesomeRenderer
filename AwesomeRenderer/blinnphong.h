#ifndef _BLINN_PHONG_H_
#define _BLINN_PHONG_H_

#include "bxdf.h"

namespace AwesomeRenderer
{
	class RenderContext;

	namespace RayTracing
	{

		class BlinnPhong : public BxDF
		{

		public:
			BlinnPhong();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;

		};


	}
}

#endif