#include "awesomerenderer.h"
#include "material.h"

using namespace AwesomeRenderer;


Material::Material() : diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
	diffuseColor(Color::WHITE), specularColor(Color::BLACK),
	shininess(0.0f), shader(NULL), translucent(FALSE)
{

}