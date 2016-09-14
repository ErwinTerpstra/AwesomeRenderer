#ifndef _BLINN_PHONG_H_
#define _BLINN_PHONG_H_

#include "bsdf.h"

namespace AwesomeRenderer
{

	namespace RayTracing
	{

		class BlinnPhong : public BSDF
		{

		public:
			BlinnPhong();

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material);

		};


	}
}

#endif