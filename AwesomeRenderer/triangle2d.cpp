#include "awesomerenderer.h"

#include "triangle2d.h"

using namespace AwesomeRenderer;

Triangle2D::Triangle2D(const Vector2& a, const Vector2& b, const Vector2& c)
	:Triangle(a, b, c)
{
	
}

Triangle2D::Triangle2D(const Triangle2D& other)
	: Triangle(other)
{

}

Triangle2D& Triangle2D::operator=(const Triangle2D& rhs)
{
	CopyFrom(rhs);

	return *this;
}