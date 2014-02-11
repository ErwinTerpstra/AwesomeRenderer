#include "awesomerenderer.h"


using namespace AwesomeRenderer;


Material::Material() : diffuseMap(NULL), specularMap(NULL), normalMap(NULL),
	ambientColor(Color::BLACK), diffuseColor(Color::WHITE), specularColor(Color::BLACK),
	shininess(0.0f), shader(NULL)
{

}