#ifndef _SAMPLE_UTIL_H_
#define _SAMPLE_UTIL_H_

namespace AwesomeRenderer
{
	class SampleUtil
	{

	public:

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

	};
}

#endif