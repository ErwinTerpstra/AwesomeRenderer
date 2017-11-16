#ifndef _BXDF_H_
#define _BXDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class RenderContext;
	class Material;
	struct RaycastHit;

	namespace RayTracing
	{

		class BxDF
		{
			

		public:

			BxDF();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext) const = 0;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const = 0;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const = 0;
			
		};
	}
}

#endif