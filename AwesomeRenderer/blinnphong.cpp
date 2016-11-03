#include "stdafx.h"
#include "blinnphong.h"

#include "material.h"
#include "phongmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BlinnPhong::BlinnPhong()
{

}

Vector3 BlinnPhong::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	Vector3 halfVector = cml::normalize(wo + wi);
	float specularTerm = std::pow(std::max(cml::dot(normal, halfVector), 0.0f), phongMaterial->shininess);
	float normalization = (phongMaterial->shininess + 8) / (8 * PI);
	
	return (phongMaterial->specularColor * normalization * specularTerm).subvector(3);
}

void BlinnPhong::GenerateSampleVector(const Vector2& r, const Material& material, float& phi, float& theta, float& pdf) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	phi = 2.0f * PI * r[0];
	theta = acosf(pow(1.0f - r[1], 1.0f / (phongMaterial->shininess + 1.0f)));

	pdf = (phongMaterial->shininess + 1) / (2.0f * PI) * pow(cosf(theta), phongMaterial->shininess) * sinf(theta);
}