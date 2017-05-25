#include "stdafx.h"

#include "arealight.h"
#include "shape.h"
#include "primitive.h"

using namespace AwesomeRenderer;

AreaLight::AreaLight()
{

}

AreaLight::~AreaLight()
{

}

const Primitive& AreaLight::GetPrimitive() const 
{
	return *primitive; 
}

const Shape& AreaLight::GetShape() const
{
	return *static_cast<const Shape*>(primitive); 
}