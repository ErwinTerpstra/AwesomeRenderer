#ifndef _BLINN_PHONG_H_
#define _BLINN_PHONG_H_

#include "bxdf.h"

namespace AwesomeRenderer
{

	namespace RayTracing
	{

		class BlinnPhong : public BxDF
		{

		public:
			BlinnPhong();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const;
			virtual void GenerateSampleVector(const Vector2& r, const Material& material, float& phi, float& theta, float& pdf) const;

		};


	}
}

#endif