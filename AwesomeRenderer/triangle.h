#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

namespace AwesomeRenderer
{

	template <typename VectorType>
	class Triangle
	{
	
	protected:
		// The three positions making this triangle
		VectorType v[3];

		// Precalculated values for barycentric coordinates
		VectorType v0, v1;
		float dot00, dot01, dot11, invDenom;

	public:
		Triangle(const VectorType& a, const VectorType& b, const VectorType& c)
		{
			v[0] = a; v[1] = b; v[2] = c;
		}

		Triangle(const Triangle<VectorType>& other) : Triangle(other.v[0], other.v[1], other.v[1])
		{

		}
		
		const VectorType& operator[](int idx)
		{
			return v[idx];
		}

		void CalculateBarycentricCoords(const VectorType& p, Vector3& barycentricCoords)
		{
			VectorType v2 = p - v[0];

			// Compute dot products
			float dot02 = cml::dot(v0, v2);
			float dot12 = cml::dot(v1, v2);

			// Compute barycentric coordinates
			float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
			float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

			// Save calculated coord
			barycentricCoords.set(1.0f - u - v, v, u);
		}

		bool IsPointInside(const VectorType& p, Vector3& barycentricCoords)
		{
			CalculateBarycentricCoords(p, barycentricCoords);

			// Check if point is in triangle
			return (barycentricCoords[0] >= 0.0f) && (barycentricCoords[1] >= 0.0f) && (barycentricCoords[2] >= 0.0f);
		}

		void PreCalculateBarycentric()
		{
			// Compute vectors from A -> C and A -> B
			v0 = v[2] - v[0];
			v1 = v[1] - v[0];

			dot00 = cml::dot(v0, v0);
			dot01 = cml::dot(v0, v1);
			dot11 = cml::dot(v1, v1);
			invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		}

	};

}

#endif