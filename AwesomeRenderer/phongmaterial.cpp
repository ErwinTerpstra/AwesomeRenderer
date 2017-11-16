#include "stdafx.h"
#include "phongmaterial.h"

#include "bsdf.h"
#include "lambert.h"
#include "blinnphong.h"

using namespace AwesomeRenderer;

RayTracing::BSDF PhongMaterial::bsdf(new RayTracing::Lambert(), new RayTracing::BlinnPhong());

PhongMaterial::PhongMaterial(Material& material) : Extension(material), 
diffuseMap(NULL), specularMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{
	provider.bsdf = &bsdf;
}

PhongMaterial::~PhongMaterial()
{

}