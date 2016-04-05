#include "pbrmaterial.h"

using namespace AwesomeRenderer;

PbrMaterial::PbrMaterial() : roughness(0.5f), metallic(0.0f), ior(1.0f), albedo(Color::WHITE), specular(Color::BLACK)
{

}

PbrMaterial::~PbrMaterial()
{

}