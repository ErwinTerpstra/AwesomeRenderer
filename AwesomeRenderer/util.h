#ifndef _UTIL_H_
#define _UTIL_H_

namespace AwesomeRenderer
{
	
	template<int Size>
	struct VectorUtil
	{
		typedef cml::vector<float, cml::fixed<Size> > Vector;

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
			out = v - 2 * cml::dot(v, normal) * normal;
			
			//float d = 2 * cml::dot(v, -normal);
			//out = v + normal * d;
		}

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
	};

}

#endif