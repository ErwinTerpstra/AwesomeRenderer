#include "stdafx.h"
#include "pbrmaterial.h"

#include "microfacetspecular.h"

using namespace AwesomeRenderer;

RayTracing::MicrofacetSpecular bsdf;

PbrMaterial::PbrMaterial(Material& material) : Extension(material),
	roughness(0.5f), metallic(0.0f), ior(1.0f), albedo(Color::WHITE), specular(Color::BLACK)
{
	provider.bsdf = &bsdf;
}

PbrMaterial::~PbrMaterial()
{

}