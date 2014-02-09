#ifndef _UTIL_H_
#define _UTIL_H_

namespace AwesomeRenderer
{
	
	template<int Size>
	struct VectorUtil
	{
		typedef cml::vector<float, cml::fixed<Size> > Vector;

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
	};

}

#endif