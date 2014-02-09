#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Ray::Ray() : origin(), direction()
{

}

Ray::Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction)
{

}