#ifndef _BSDF_H_
#define _BSDF_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Material;

	namespace RayTracing
	{

		class BSDF
		{
			

		public:

			virtual Vector3 Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) = 0;

		};
	}
}

#endif