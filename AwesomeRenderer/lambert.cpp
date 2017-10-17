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
	MicrofacetMaterial* pbrMaterial = material.As<MicrofacetMaterial>();

	if (pbrMaterial != NULL)
	{
		Color result = pbrMaterial->albedo;
		
		if (pbrMaterial->albedoMap != NULL)
			result *= pbrMaterial->albedoMap->SampleMipMaps(hitInfo.uv, hitInfo.distance, hitInfo.texelToSurfaceAreaRatio, renderContext.renderTarget->frameBuffer->GetResolution());
		
		return result.subvector(3) * (1.0f - pbrMaterial->metallic) * INV_PI;
	}

	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	if (phongMaterial != NULL)
	{
		Color result = phongMaterial->diffuseColor;

		if (phongMaterial->diffuseMap != NULL)
			result *= phongMaterial->diffuseMap->SampleMipMaps(hitInfo.uv, hitInfo.distance, hitInfo.texelToSurfaceAreaRatio, renderContext.renderTarget->frameBuffer->GetResolution());

		return result.subvector(3) * INV_PI;
	}

	return Vector3(0.0f, 0.0f, 0.0f);
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