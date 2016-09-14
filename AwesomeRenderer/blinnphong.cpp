#include "stdafx.h"
#include "blinnphong.h"

#include "material.h"
#include "phongmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BlinnPhong::BlinnPhong()
{

}

Vector3 BlinnPhong::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material)
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	Vector3 halfVector = cml::normalize(wo + wi);
	float specularTerm = std::pow(std::max(cml::dot(normal, halfVector), 0.0f), phongMaterial->shininess);

	return (phongMaterial->diffuseColor + phongMaterial->specularColor * specularTerm).subvector(3);
}