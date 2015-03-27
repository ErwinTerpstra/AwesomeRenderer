#include "material.h"

using namespace AwesomeRenderer;


Material::Material() : shader(NULL), translucent(FALSE), diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
diffuseColor(Color::WHITE), specularColor(Color::BLACK),
shininess(0.0f)
{

}