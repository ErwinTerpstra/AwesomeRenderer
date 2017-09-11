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
		
	//if (NoV == 0.0f || NoL == 0.0f)
		//return Vector3(0.0f, 0.0f, 0.0f);

	Vector3 fresnel = FresnelSchlick(NoV, specular.subvector(3));

	float e = phongMaterial->shininess;

	float specularTerm = std::powf(NoH, e);
	
	float normalization = 1.0f;

	normalization = Util::Clamp(((e + 2) * (e + 4)) / (8 * PI * (powf(2, -e / 2) + e)), (e + 6) / (8 * PI), (e + 8) / (8 * PI));

	// This is the correct normalization form to use when using (N dot H) ^ shininess
	//normalization = (e + 8) / (8 * PI);

	// This is the correct normalization form to use when using (V dot R) ^ shininess
	// It is also used when using Blinn-Phong as an NDF
	//normalization = (e + 2) * INV_TWO_PI;

	// Also not sure if the denominator is neccesary for normalization. It is present in the Microfacet BRDF so it probably should be present here?
	// It makes the reflection intensity look better for smooth surfaces but might be inaccurate for the lack of the implicit geometry term
	Vector3 result = (fresnel * (normalization * specularTerm));

	result[0] = std::max(result[0], 0.0f);
	result[1] = std::max(result[1], 0.0f);
	result[2] = std::max(result[2], 0.0f);

	return result;
}

void BlinnPhong::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	float phi = 2.0f * PI * r[1];
	float theta = acosf(powf(r[0], 1.0f / (phongMaterial->shininess + 1.0f)));

	Vector3 h;
	VectorUtil<3>::SphericalToCartesian(phi, theta, h);
	VectorUtil<3>::TransformSampleVector(normal, h, h);
	
	VectorUtil<3>::Reflect(wo, h, wi);
}

float BlinnPhong::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	Vector3 h;
	if (!VectorUtil<3>::CalculateHalfVector(wo, wi, h))
		return 0.0;

	float NoH = std::max(VectorUtil<3>::Dot(normal, h), 0.0f);
	float HoV = VectorUtil<3>::Dot(wo, h);

	if (HoV <= 0.0f)
		return 0.0;

	float pdf = ((phongMaterial->shininess + 1) * powf(NoH, phongMaterial->shininess)) * INV_TWO_PI;
	return pdf / (4.0f * HoV);
}