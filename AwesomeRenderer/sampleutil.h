#ifndef _SAMPLE_UTIL_H_
#define _SAMPLE_UTIL_H_

#include "random.h"

namespace AwesomeRenderer
{
	class SampleUtil
	{

	public:
		
		static void UniformSampleDisc(const Vector2& r, Vector2& p)
		{
			float d = sqrtf(r[0]);
			float theta = r[1] * TWO_PI;
			p[0] = d * cosf(theta);
			p[1] = d * sinf(theta);
		}

		static void UniformHemisphere(const Vector2& r, float& phi, float& theta)
		{
			phi = 2 * PI * r[1];
			theta = acosf(sqrtf(std::max(0.0f, 1 - r[0] * r[0])));
		}

		static float UniformHemispherePDF()
		{
			return INV_TWO_PI;
		}

		static void CosineWeightedHemisphere(const Vector2& r, float& phi, float& theta)
		{
			phi = 2 * PI * r[1];
			theta = acosf(sqrtf(std::max(0.0f, 1 - sqrtf(r[0]))));
		}

		static float CosineWeightedHemispherePDF(const Vector3& n, const Vector3& wi)
		{
			return VectorUtil<3>::Dot(n, wi) * INV_PI;
		}

		static float StratifiedSample(uint32_t x, uint32_t n)
		{
			return StratifiedSample(x, n, Random::instance);
		}

		static float StratifiedSample(uint32_t x, uint32_t n, Random& random)
		{
			float step = 1.0f / n;
			return (x * step) + random.NextFloat() * step;
		}

	};
}

#endif