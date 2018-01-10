
#include "softwareshader.h"

using namespace AwesomeRenderer;

SoftwareShader::SoftwareShader() : Shader()
{

}

void SoftwareShader::Prepare()
{
	screenMtx = modelMtx * viewMtx * projMtx;
}