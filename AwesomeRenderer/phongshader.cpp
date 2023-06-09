
#include "phongshader.h"
#include "sampler.h"
#include "material.h"
#include "phongmaterial.h"
#include "lightdata.h"

using namespace AwesomeRenderer;

PhongShader::PhongShader() : SoftwareShader()
{

}

void PhongShader::ProcessVertex(const VertexInfo& in, VertexToPixel& out) const
{
	Vector4 position(in.position, 1.0f);

	out.worldPosition = position * modelMtx;
	out.screenPosition = position * screenMtx;
	
	Vector4 normal(in.normal, 0.0f);
	normal = normal * modelMtx;
	out.normal = normal.subvector(3).normalize();
	
	out.color = in.color;

	out.uv = in.uv;
}

void PhongShader::ProcessPixel(const VertexToPixel& in, PixelInfo& out) const
{
	const PhongMaterial* material = this->material->As<PhongMaterial>();

	Color diffuse = material->diffuseColor;
	Color specular = material->specularColor;
	
	float shininess = material->shininess;

	// Sample diffuse map if it is present
	if (material->diffuseMap)
		diffuse *= material->diffuseMap->Sample(in.uv);

	// Sample specular map if it is present
	if (material->specularMap)
	{
		Color sample = material->specularMap->Sample(in.uv);
		
		specular *= sample;		// Multiply specular color with the color for this pixel
		shininess *= sample[3];	// Multiply global shininess with the local value for this pixel
	}

	/*
	Color diffuseLight = Color::WHITE;
	Color specularLight = Color::BLACK;
	/*/
	Color diffuseLight = Color::BLACK;
	Color specularLight = Color::BLACK;

	// Iterate through all the lights
	for (uint8_t i = 0; i < LightData::MAX_LIGHTS; ++i)
	{
		const LightData::Light& light = lightData->lights[i];

		if (!light.enabled)
			continue;

		// Calculate light intensity
		Vector3 toLight;
		float intensity = light.intensity;

		if (light.type != LightData::DIRECTIONAL)
		{
			toLight = light.position - in.worldPosition.subvector(3);
			float distanceToLight = toLight.length();
			toLight.normalize();

			if (light.type == LightData::SPOT)
			{
				float angleTerm = VectorUtil<3>::Dot(light.direction, -toLight);
				float cosAngle = cos(light.angle);

				if (angleTerm > cosAngle)
					intensity *= (angleTerm - cosAngle) / (1.0f - cosAngle);
				else
					intensity = 0;
			}

			intensity *= 1.0f / (light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight));
		}
		else
			toLight = -light.direction;

		
		// Compute the diffuse term
		float diffuseTerm = std::max(VectorUtil<3>::Dot(in.normal, toLight), 0.0f);
		diffuseLight += light.color * diffuseTerm * intensity;

		// Compute the specular term
		if (diffuseTerm > 0.0f)
		{
			Vector3 toEye = cml::normalize(viewPosition - in.worldPosition).subvector(3);
			Vector3 halfVector = cml::normalize(toLight + toEye);

			float specularTerm = std::pow(std::max(VectorUtil<3>::Dot(in.normal, halfVector), 0.0f), shininess);
			specularLight += light.color * specularTerm * intensity;
		}
	}
	//*/
	
	out.color = diffuse * (lightData->ambient + diffuseLight) + specular * specularLight;
}