#include "awesomerenderer.h"

using namespace AwesomeRenderer;

SoftwareShader::SoftwareShader() : Shader()
{

}

void SoftwareShader::Prepare()
{
	screenMtx = modelMtx * viewMtx * projMtx;
	viewPosition = viewMtx * Vector4();
}