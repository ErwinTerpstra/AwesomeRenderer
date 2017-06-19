#include "stdafx.h"
#include "blinnphong.h"

#include "material.h"
#include "phongmaterial.h"

#include "raycasthit.h"
#include "sampler.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BlinnPhong::BlinnPhong()
{

}

Vector3 BlinnPhong::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	Color specular = phongMaterial->specularColor;
	if (phongMaterial->specularMap != NULL)
		specular *= phongMaterial->specularMap->Sample(hitInfo.uv);

	Vector3 halfVector = cml::normalize(wo + wi);
	float NoV = std::max(VectorUtil<3>::Dot(normal, wo), 0.0f);
	float NoL = std::max(VectorUtil<3>::Dot(normal, wi), 0.0f);
	float NoH = std::max(VectorUtil<3>::Dot(normal, halfVector), 0.0f);

	if (NoV == 0.0f || NoL == 0.0f)
		return Vector3(0.0f, 0.0f, 0.0f);

	Vector3 fresnel = FresnelSchlick(NoV, specular.subvector(3));

	float specularTerm = std::powf(NoH, phongMaterial->shininess);
	
	float normalization = (phongMaterial->shininess + 2) * INV_TWO_PI;

	// Not sure why some sources cite this as the correct normalization factor for Blinn specular, it seems too dark
	//float normalization = (phongMaterial->shininess + 8) / (8 * PI);

	// Also not sure if the denominator is neccesary for normalization. It is present in the Microfacet BRDF so it probably should be present here?
	// It makes the reflection intensity look better for smooth surfaces but might be inaccurate for the lack of the implicit geometry term
	return (fresnel * (normalization * specularTerm)) / 4;// / (4 * NoL * NoV);
}

void BlinnPhong::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	float phi = 2.0f * PI * r[1];
	float theta = acosf(powf(r[0], 1.0f / (phongMaterial->shininess + 1.0f)));

	Vector3 h;
	SphericalToCartesian(phi, theta, h);
	TransformSampleVector(normal, h, h);
	
	VectorUtil<3>::Reflect(wo, h, wi);
}

float BlinnPhong::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	Vector3 h = cml::normalize(wo + wi);
	float cosTheta = VectorUtil<3>::Dot(normal, h);

	float HoV = VectorUtil<3>::Dot(wo, h);
	if (HoV <= 0.0f)
		return 0.0f;

	float pdf = ((phongMaterial->shininess + 1) * powf(cosTheta, phongMaterial->shininess)) * INV_TWO_PI;
	return pdf / (4.0f * HoV);
}