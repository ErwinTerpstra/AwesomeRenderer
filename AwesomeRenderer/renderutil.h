#ifndef _RENDER_UTIL_H_
#define _RENDER_UTIL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class RenderUtil
	{

	public:

		static float Fresnel(const Vector3& v, const Vector3& normal, float ior)
		{
			float cosi = VectorUtil<3>::Dot(v, normal);
			float etai = 1, etat = ior;

			if (cosi > 0)
				std::swap(etai, etat);

			// Compute sini using Snell's law
			float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));

			// Total internal reflection
			if (sint >= 1)
				return 1.0f;

			float cost = sqrtf(std::max(0.f, 1 - sint * sint));
			cosi = fabsf(cosi);

			float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
			float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

			return (Rs * Rs + Rp * Rp) / 2;
		}

		template<typename T>
		static T FresnelSchlick(float cosT, T F0)
		{
			return F0 + (1.0f - F0) * pow(1 - cosT, 5);
		};
	};
}

#endif