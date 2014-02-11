#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Shader::Shader()
{

}

void Shader::Prepare()
{
	screenMtx = modelMtx * viewMtx * projMtx;
	viewPosition = viewMtx * Vector4();
}