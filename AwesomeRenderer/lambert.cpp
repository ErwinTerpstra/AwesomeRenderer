#include "stdafx.h"
#include "lambert.h"

#include "material.h"
#include "phongmaterial.h"
#include "sampler.h"

#include "rendercontext.h"
#include "texture.h"

#include "microfacetmaterial.h"
#include "raycasthit.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

Lambert::Lambert()
{

}

Vector3 Lambert::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext) const
{
	return SampleAlbedo(hitInfo, material, renderContext).subvector(3) * INV_PI;
}

void Lambert::GenerateSampleVector(const Vector2& r, const Vector3& wo, const Vector3& normal, const Material& material, Vector3& wi) const
{
	float phi, theta;
	SampleUtil::CosineWeightedHemisphere(r, phi, theta);

	Vector3 v;
	VectorUtil<3>::SphericalToCartesian(phi, theta, v);
	VectorUtil<3>::TransformSampleVector(normal, v, wi);
}

float Lambert::CalculatePDF(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	return SampleUtil::CosineWeightedHemispherePDF(normal, wi);
}

Color Lambert::SampleAlbedo(const RaycastHit& hitInfo, const Material& material, const RenderContext& renderContext)
{
	Color albedo = Color::BLACK;
	Sampler* albedoMap = NULL;

	PhongMaterial* phongMaterial = material.As<PhongMaterial>();
	MicrofacetMaterial* microfacetMaterial = material.As<MicrofacetMaterial>();
	
	if (phongMaterial != NULL)
	{
		albedo = phongMaterial->diffuseColor;
		albedoMap = phongMaterial->diffuseMap;
	}

	if (microfacetMaterial != NULL)
	{
		albedo = microfacetMaterial->albedo;
		albedoMap = microfacetMaterial->albedoMap;

		albedo = Color(albedo.subvector(3) *  (1.0f - microfacetMaterial->metallic), albedo[3]);
	}

	if (albedoMap != NULL)
		albedo *= albedoMap->SampleMipMaps(hitInfo.uv, hitInfo.distance, hitInfo.surfaceAreaToTextureRatio, renderContext.renderTarget->frameBuffer->GetResolution());

	return albedo;
}