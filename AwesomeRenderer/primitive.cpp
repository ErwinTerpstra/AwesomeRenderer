
#include "awesomerenderer.h"

#include "primitive.h"
#include "plane.h"

using namespace AwesomeRenderer;

Primitive::Primitive()
{

}


int Primitive::SideOfAAPlane(int axis, float position) const
{
	Vector3 normal(0.0f, 0.0f, 0.0f);
	normal[axis] = 1.0f;

	Plane plane(position, normal);
	return SideOfPlane(plane);
}