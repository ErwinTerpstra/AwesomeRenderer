#include "stdafx.h"
#include "material.h"

using namespace AwesomeRenderer;


Material::Material() : shader(NULL), bsdf(NULL), translucent(FALSE), emission(Color::BLACK), ior(1.0f)
{

}