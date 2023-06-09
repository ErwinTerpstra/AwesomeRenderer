#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

namespace AwesomeRenderer
{
	template <typename VectorType>
	class Triangle
	{
	
	public:
		
		// The three positions making this triangle
		VectorType v[3]; // 36 bytes

		// Precalculated values for barycentric coordinates
		VectorType v0, v1; // 24 bytes
		float dot00, dot01, dot11, invDenom; // 16 bytes

	public:
		Triangle(const VectorType& a, const VectorType& b, const VectorType& c)
		{
			v[0] = a; v[1] = b; v[2] = c;
		}

		Triangle(const Triangle<VectorType>& other)
		{
			CopyFrom(other);
		}

		Triangle& operator=(const Triangle<VectorType>& rhs)
		{
			CopyFrom(rhs);

			return *this;
		}

		void CopyFrom(const Triangle<VectorType>& other)
		{
			v[0] = other.v[0];
			v[1] = other.v[1];
			v[2] = other.v[2];

			v0 = other.v0;
			v1 = other.v1;

			dot00 = other.dot00;
			dot01 = other.dot01;
			dot11 = other.dot11;

			invDenom = other.invDenom;
		}
				
		const VectorType& operator[](int idx) const
		{
			return v[idx];
		}

		void CalculateBarycentricCoords(const VectorType& p, Vector3& barycentricCoords) const
		{
			VectorType v2 = p - v[0];

			// Compute dot products
			float dot02 = VectorUtil<VectorType::dimension>::Dot(v0, v2);
			float dot12 = VectorUtil<VectorType::dimension>::Dot(v1, v2);

			// Compute barycentric coordinates
			float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			// Save calculated coord
			barycentricCoords.set(1.0f - u - v, u, v);
		}

		bool IsPointInside(const VectorType& p, Vector3& barycentricCoords) const
		{
			CalculateBarycentricCoords(p, barycentricCoords);

			// Check if point is in triangle
			return (barycentricCoords[0] >= 0.0f) && (barycentricCoords[1] >= 0.0f) && (barycentricCoords[2] >= 0.0f);
		}

		bool IsLine() const
		{
			VectorType v0 = VectorUtil<VectorType::dimension>::Normalize(v[1] - v[0]);
			VectorType v1 = VectorUtil<VectorType::dimension>::Normalize(v[2] - v[0]);

			return (1.0f - fabs(VectorUtil<VectorType::dimension>::Dot(v0, v1))) < 1e-5f;
		}

		void PreCalculateBarycentric()
		{ 
			// Compute vectors from A -> C and A -> B
			v0 = v[1] - v[0];
			v1 = v[2] - v[0];

			dot00 = VectorUtil<VectorType::dimension>::Dot(v0, v0);
			dot01 = VectorUtil<VectorType::dimension>::Dot(v0, v1);
			dot11 = VectorUtil<VectorType::dimension>::Dot(v1, v1);

			float denom = dot00 * dot11 - dot01 * dot01;

			if (denom < 1e-15f)
				invDenom = 1.0;
			else
				invDenom = 1.0f / denom;
		}

		void CalculateBounds(VectorType& lower, VectorType& upper) const
		{
			for (int i = 0; i < VectorType::dimension; ++i)
			{
				lower[i] = FLT_MAX;
				upper[i] = -FLT_MAX;

				for (int cVertex = 0; cVertex < 3; ++cVertex)
				{
					lower[i] = std::min(v[cVertex][i], lower[i]);
					upper[i] = std::max(v[cVertex][i], upper[i]);
				}
			}
		}

		static float Area(const VectorType& v0, const VectorType& v1, const VectorType& v2)
		{
			/*
			float a = VectorUtil<VectorType::dimension>::Dot(v0, v0);
			float b = VectorUtil<VectorType::dimension>::Dot(v1, v1);
			float c = VectorUtil<VectorType::dimension>::Dot(v2, v2);

			return sqrtf(((2 * a * b) + (2 * b * c) + (2 * c * a) - (a * a) - (b * b) - (c * c)) / 16);
			*/

			float a = (v1 - v0).length();
			float b = (v2 - v0).length();
			float c = (v2 - v1).length();

			float s = 0.5f * (a + b + c);

			return sqrtf(s * (s - a) * (s - b) * (s - c));
		}

	};

}

#endif