#ifndef _LAMBERT_H_
#define _LAMBERT_H_

#include "bxdf.h"

namespace AwesomeRenderer
{

	namespace RayTracing
	{

		class Lambert : public BxDF
		{

		public:
			Lambert();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;
			virtual void GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const;
			virtual float CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;
		};


	}
}

#endif