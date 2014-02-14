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