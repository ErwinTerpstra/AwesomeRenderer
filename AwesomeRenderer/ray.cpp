
#include "awesomerenderer.h"

#include "ray.h"

using namespace AwesomeRenderer;

Ray::Ray()
{

}

Ray::Ray(const Vector3& origin, const Vector3& direction) : 
	origin(origin), direction(cml::normalize(direction)), type(UNKOWN)
{
	invDirection.set(
		1.0f / this->direction[0],
		1.0f / this->direction[1],
		1.0f / this->direction[2]);
}