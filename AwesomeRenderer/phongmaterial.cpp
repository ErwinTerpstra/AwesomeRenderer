#include "stdafx.h"
#include "phongmaterial.h"

#include "bsdf.h"
#include "lambert.h"
#include "blinnphong.h"

using namespace AwesomeRenderer;

RayTracing::BSDF phongBSDF(/*new RayTracing::Lambert()*/ NULL, new RayTracing::BlinnPhong());

PhongMaterial::PhongMaterial(Material& material) : Extension(material), 
diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{
	provider.bsdf = &phongBSDF;
}

PhongMaterial::~PhongMaterial()
{

}