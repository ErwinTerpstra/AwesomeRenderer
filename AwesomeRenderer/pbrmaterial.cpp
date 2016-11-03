#include "stdafx.h"
#include "pbrmaterial.h"


#include "bsdf.h"
#include "lambert.h"
#include "microfacetspecular.h"

using namespace AwesomeRenderer;

RayTracing::BSDF pbrBSDF(new RayTracing::Lambert(), new RayTracing::MicrofacetSpecular());

PbrMaterial::PbrMaterial(Material& material) : Extension(material),
	roughness(0.5f), metallic(0.0f), albedo(Color::WHITE), specular(Color::BLACK)
{
	provider.bsdf = &pbrBSDF;
}

PbrMaterial::~PbrMaterial()
{

}