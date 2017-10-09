#ifndef _VECTOR_UTIL_H_
#define _VECTOR_UTIL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	template<int Size>
	struct VectorUtilBase
	{
		typedef cml::vector<float, cml::fixed<Size> > Vector;

		AR_FORCE_INLINE static Vector Normalize(const Vector& v)
		{
			return cml::normalize(v);
		}

		AR_FORCE_INLINE static bool IsNormalized(const Vector& v)
		{
			return abs(v.length_squared() - 1.0f) < 1e-5f;
		}

		AR_FORCE_INLINE static float Dot(const Vector& a, const Vector& b)
		{
			return cml::dot(a, b);
		}

		AR_FORCE_INLINE static void Interpolate(const Vector& a, const Vector& b, float d, Vector& out)
		{
			out = a * (1.0f - d) + b * d;
		}

		AR_FORCE_INLINE static void Interpolate(const Vector& a, const Vector& b, const Vector& c,
			const Vector3& barycentricCoords, Vector& out)
		{
			for (int i = 0; i < Size; ++i)
			{
				out[i] = (a[i] * barycentricCoords[0]) +
					(b[i] * barycentricCoords[1]) +
					(c[i] * barycentricCoords[2]);
			}
		}

		AR_FORCE_INLINE static bool CalculateHalfVector(const Vector3& wo, const Vector3& wi, Vector3& h)
		{
			h = wo + wi;

			if (h.length_squared() < 1e-5f)
				return false;

			h = Normalize(h);

			return true;
		}

		static Vector Max(const Vector& v, float x)
		{
			Vector result;

			for (int i = 0; i < Size; ++i)
				result[i] = std::max(v[i], x);

			return result;
		}

		static Vector Min(const Vector& v, float x)
		{
			Vector result;

			for (int i = 0; i < Size; ++i)
				result[i] = std::min(v[i], x);

			return result;
		}
	};

	template<int Size>
	struct VectorUtil : public VectorUtilBase<Size> { };

	template<>
	struct VectorUtil<3> : public VectorUtilBase<3>
	{
		static void OrthoNormalize(const Vector3& up, Vector3& right, Vector3& forward)
		{
			if (std::fabs(up[0]) > std::fabs(up[1]))
				right = Vector3(up[2], 0, -up[0]) / sqrtf(up[0] * up[0] + up[2] * up[2]);
			else
				right = Vector3(0, -up[2], up[1]) / sqrtf(up[1] * up[1] + up[2] * up[2]);

			forward = cml::normalize(cml::cross(up, right));
		}

		static void OrthoNormalize(const Vector3& up, const Vector3& forwardHint, Vector3& right, Vector3& forward)
		{
			if (1.0f - std::fabs(Dot(up, forwardHint)) < 1e-5f)
			{
				OrthoNormalize(up, right, forward);
				return;
			}

			right = cml::normalize(cml::cross(forwardHint, up));
			forward = cml::normalize(cml::cross(up, right));
		}

		static void Reflect(const Vector3& v, const Vector3& normal, Vector3& out)
		{
			out = -v + 2 * Dot(v, normal) * normal;
		}

		static void Refract(const Vector3& v, const Vector3& normal, float ior, Vector3& out)
		{
			assert(IsNormalized(v));
			assert(IsNormalized(normal));

			float cosi = Dot(v, normal);
			float etai = 1, etat = ior;
			Vector3 n = normal;

			if (cosi < 0)
			{
				cosi = -cosi;
			}
			else
			{
				std::swap(etai, etat);
				n = -normal;
			}

			float eta = etai / etat;
			float k = 1 - eta * eta * (1 - cosi * cosi);

			if (k >= 0)
			{
				out = eta * v + (eta * cosi - sqrtf(k)) * n;
				assert(IsNormalized(out));
			}
			else
				out = Vector3(0.0f, 0.0f, 0.0f);
		}

		AR_FORCE_INLINE static float Dot(const Vector3& a, const Vector3& b)
		{
			return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
		}


		static void SphericalToCartesian(float phi, float theta, Vector3& v)
		{
			// Convert to cartesian coordinates
			float sinTheta = sinf(theta);
			float x = sinTheta * cosf(phi);
			float z = sinTheta * sinf(phi);

			v = Vector3(x, cosf(theta), z);

			assert(IsNormalized(v));
		}

		static void TransformSampleVector(const Vector3& n, const Vector3& in, Vector3& out)
		{
			assert(IsNormalized(in));
			assert(IsNormalized(n));

			// Create an orientation matrix that aligns with the surface normal
			Vector3 right, forward;
			//OrthoNormalize(n, in, right, forward);
			OrthoNormalize(n, right, forward);

			assert(IsNormalized(right));
			assert(IsNormalized(forward));

			assert(fabs(Dot(n, forward)) < 1e-5f);
			assert(fabs(Dot(n, right)) < 1e-5f);
			assert(fabs(Dot(right, forward)) < 1e-5f);

			Matrix33 transform(
				right[0], right[1], right[2],
				n[0], n[1], n[2],
				forward[0], forward[1], forward[2]
			);

			// Transform the sample to world space
			out = transform_vector(transform, in);

			assert(IsNormalized(out));
		}
	};

}

#endif