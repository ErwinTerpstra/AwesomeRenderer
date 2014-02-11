#include "awesomerenderer.h"

using namespace AwesomeRenderer;

PhongShader::PhongShader() : Shader()
{

}

void PhongShader::ProcessVertex(const VertexInfo& in, VertexToPixel& out) const
{
	Vector4 position(in.position, 1.0f);

	out.worldPosition = position * modelMtx;
	out.screenPosition = position * screenMtx;
	out.normal = in.normal;
	out.color = in.color;

	out.uv = in.uv;
}

void PhongShader::ProcessPixel(const VertexToPixel& in, PixelInfo& out) const
{
	Color diffuse = material->diffuseColor;
	Color specular = material->specularColor;
	
	float shininess = material->shininess;

	// Sample diffuse map if it is present
	if (material->diffuseMap)
		diffuse *= material->diffuseMap->Sample(in.uv);

	// Sample specular map if it is present
	if (material->specularMap)
	{
		Color sample = material->diffuseMap->Sample(in.uv);
		
		specular *= sample;
		shininess *= sample[3];	// Multiply global shininess with the local value for this pixel
	}

	Color diffuseLight = Color::BLACK;
	Color specularLight = Color::BLACK;

	// Iterate through all the lights
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		const Light& light = lightData.lights[i];

		if (!light.enabled)
			break;

		// Calculate light intensity
		Vector3 toLight = light.position - in.worldPosition.subvector(3);
		float distanceToLight = toLight.length();
		toLight.normalize();

		float attenuation = light.constantAttenuation + (light.lineairAttenuation * distanceToLight) + (light.quadricAttenuation * distanceToLight * distanceToLight);
		float intensity = light.intensity * (1.0f / attenuation);

		// Compute the diffuse term
		float diffuseTerm = std::max(cml::dot(in.normal, toLight), 0.0f);

		diffuseLight += light.color * diffuseTerm * intensity;

		// Compute the specular term
		Vector3 toEye = cml::normalize(viewPosition - in.worldPosition).subvector(3);
		Vector3 specularDirection = cml::normalize(toLight + toEye);

		float specularTerm = std::pow(std::max(cml::dot(in.normal, specularDirection), 0.0f), material->shininess);

		if (diffuseTerm <= 0.0f) 
			specularTerm = 0.0f;

		specularLight += light.color * specularTerm * intensity;
	}

	// Ambient Term
	Color ambient = material->ambientColor * lightData.ambient;
	
	out.color = ambient + diffuse * diffuseLight + specular * specularLight;
}