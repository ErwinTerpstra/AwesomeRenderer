#include "stdafx.h"
#include "lambert.h"

#include "material.h"
#include "phongmaterial.h"
#include "pbrmaterial.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

Lambert::Lambert()
{

}

Vector3 Lambert::Sample(const Vector3& wo, const Vector3& wi, const Vector3& normal, const Material& material) const
{
	PhongMaterial* phongMaterial = material.As<PhongMaterial>();

	if (phongMaterial != NULL)
		return phongMaterial->diffuseColor.subvector(3) / PI;

	PbrMaterial* pbrMaterial = material.As<PbrMaterial>();

	if (pbrMaterial != NULL)
		return (pbrMaterial->albedo.subvector(3) * (1.0f - pbrMaterial->metallic)) / PI;

	return Vector3(0.0f, 0.0f, 0.0f);
}

void Lambert::GenerateSampleVector(const Vector2& r, const Material& material, float& phi, float& theta, float& pdf) const
{
	phi = 2 * M_PI * r[1];
	theta = acos(r[0]);

	pdf = r[0] / PI;
	//pdf = 1 / (2 * PI);
}
