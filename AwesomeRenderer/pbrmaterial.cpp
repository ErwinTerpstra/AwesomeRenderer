#include "stdafx.h"
#include "pbrmaterial.h"


#include "bsdf.h"
#include "lambert.h"
#include "microfacetspecular.h"

using namespace AwesomeRenderer;

RayTracing::BSDF PbrMaterial::metallicBSDF(NULL, new RayTracing::MicrofacetSpecular());
RayTracing::BSDF PbrMaterial::dielectricBSDF(new RayTracing::Lambert(), new RayTracing::MicrofacetSpecular());

PbrMaterial::PbrMaterial(Material& material) : PbrMaterial(dielectricBSDF, material)
{

}

PbrMaterial::PbrMaterial(RayTracing::BSDF& bsdf, Material& material) : Extension(material),
roughness(0.5f), metallic(0.0f), albedo(Color::WHITE), specular(Color::BLACK)
{
	provider.bsdf = &bsdf;
}

PbrMaterial::~PbrMaterial()
{

}