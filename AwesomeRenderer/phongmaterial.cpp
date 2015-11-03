#include "phongmaterial.h"

using namespace AwesomeRenderer;

PhongMaterial::PhongMaterial() : diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{

}