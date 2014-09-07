#include "awesomerenderer.h"

using namespace AwesomeRenderer;

SoftwareShader::SoftwareShader() : Shader()
{

}

void SoftwareShader::Prepare()
{
	screenMtx = modelMtx * viewMtx * projMtx;
	viewPosition = viewMtx * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}