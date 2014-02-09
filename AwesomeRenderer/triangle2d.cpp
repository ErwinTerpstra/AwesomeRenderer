#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Triangle2D::Triangle2D(const Vector2& a, const Vector2& b, const Vector2& c)
	:Triangle(a, b, c)
{
	
}

Triangle2D::Triangle2D(const Triangle2D& other)
	: Triangle2D(other.v[0], other.v[1], other.v[2])
{

}

void Triangle2D::CalculateBounds(Vector2& lower, Vector2& upper)
{
	lower.set(FLT_MAX, FLT_MAX);
	upper.set(FLT_MIN, FLT_MIN);

	for (int cVertex = 0; cVertex < 3; ++cVertex)
	{
		for (int i = 0; i < 2; ++i)
		{
			lower[i] = std::min(v[cVertex][i], lower[i]);
			upper[i] = std::max(v[cVertex][i], upper[i]);
		}
	}
}