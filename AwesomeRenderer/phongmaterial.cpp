#include "stdafx.h"
#include "phongmaterial.h"

#include "blinnphong.h"

using namespace AwesomeRenderer;

RayTracing::BlinnPhong bsdf;

PhongMaterial::PhongMaterial(Material& material) : Extension(material), 
diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{
	provider.bsdf = &bsdf;
}

PhongMaterial::~PhongMaterial()
{

}