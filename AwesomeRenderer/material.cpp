#include "stdafx.h"
#include "material.h"

using namespace AwesomeRenderer;


Material::Material() : shader(NULL), bsdf(NULL), translucent(FALSE), emission(Color::BLACK), emissionIntensity(0.0f), ior(1.0f)
{

}