#ifndef _UTIL_H_
#define _UTIL_H_

namespace AwesomeRenderer
{

	template<int Size>
	struct VectorUtilBase
	{
		typedef cml::vector<float, cml::fixed<Size> > Vector;

		static bool IsNormalized(const Vector& v)
		{
			return abs(v.length_squared() - 1.0f) < 1e-5f;
		}

		static void Interpolate(const Vector& a, const Vector& b, float d, Vector& out)
		{
			out = a * (1.0f - d) + b * d;
		}

		static void Interpolate(const Vector& a, const Vector& b, const Vector& c,
			const Vector3& barycentricCoords, Vector& out)
		{
			for (int i = 0; i < Size; ++i)
			{
				out[i] = (a[i] * barycentricCoords[0]) +
					(b[i] * barycentricCoords[1]) +
					(c[i] * barycentricCoords[2]);
			}
		}


		static void Reflect(const Vector& v, const Vector& normal, Vector& out)
		{
			out = -v + 2 * cml::dot(v, normal) * normal;
		}

		static void Refract(const Vector& v, const Vector& normal, float ior, Vector3& out)
		{
			assert(IsNormalized(v));
			assert(IsNormalized(normal));

			float cosi = cml::dot(v, normal);
			float etai = 1, etat = ior;
			Vector n = normal;

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
		};

		static void OrthoNormalize(const Vector3& up, const Vector3& forwardHint, Vector3& right, Vector3& forward)
		{
			if (1.0f - std::fabs(cml::dot(up, forwardHint)) < 1e-5f)
			{
				OrthoNormalize(up, right, forward);
				return;
			}

			right = cml::normalize(cml::cross(forwardHint, up));
			forward = cml::normalize(cml::cross(up, right));
		};
	};
	
	class Util
	{

	public:

		template <typename T>
		__inline static void Sort(T& a, T& b)
		{
			if (a > b)
				Swap(a, b);
		}

		template <typename T>
		__inline static void Swap(T& a, T& b)
		{
			T tmp = a;
			a = b;
			b = tmp;
		}


		template <typename T>
		__inline static void Swap(T** a, T** b)
		{
			T* tmp = *a;
			*a = *b;
			*b = tmp;
		}

		template <typename T> 
		_inline static int32_t Sign(T val) 
		{
			return (T(0) < val) - (val < T(0));
		}

		template <typename T>
		__inline static T Clamp(T val, T min, T max)
		{
			return std::min(std::max(val, min), max);
		}

		__inline static float Clamp01(float val)
		{
			return Clamp(val, 0.0f, 1.0f);
		}
	};

}

#endif