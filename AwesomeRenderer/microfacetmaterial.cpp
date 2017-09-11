#include "stdafx.h"
#include "microfacetmaterial.h"


#include "bsdf.h"
#include "lambert.h"
#include "microfacetspecular.h"

using namespace AwesomeRenderer;

RayTracing::BSDF MicrofacetMaterial::metallicBSDF(NULL, new RayTracing::MicrofacetSpecular());
RayTracing::BSDF MicrofacetMaterial::dielectricBSDF(new RayTracing::Lambert(), new RayTracing::MicrofacetSpecular());

MicrofacetMaterial::MicrofacetMaterial(Material& material) : MicrofacetMaterial(dielectricBSDF, material)
{

}

MicrofacetMaterial::MicrofacetMaterial(RayTracing::BSDF& bsdf, Material& material) : Extension(material),
	albedoMap(NULL), specularMap(NULL), normalMap(NULL),
	roughness(0.5f), metallic(0.0f), albedo(Color::WHITE), specular(Color::BLACK)
{
	provider.bsdf = &bsdf;
}

MicrofacetMaterial::~MicrofacetMaterial()
{

}