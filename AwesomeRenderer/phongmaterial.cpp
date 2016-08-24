#include "phongmaterial.h"

using namespace AwesomeRenderer;

PhongMaterial::PhongMaterial(Material& material) : Extension(material), 
diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{

}

PhongMaterial::~PhongMaterial()
{

}