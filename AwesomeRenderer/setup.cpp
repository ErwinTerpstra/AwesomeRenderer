#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "awesomerenderer.h"

// Primitives
#include "transformation.h"
#include "shape.h"
#include "plane.h"
#include "aabb.h"
#include "sphere.h"
#include "triangle.h"
#include "triangle3d.h"
#include "triangle2d.h"
#include "node.h"

// Rendering
#include "texture.h"
#include "sampler.h"

#include "material.h"
#include "phongmaterial.h"
#include "pbrmaterial.h"

#include "mesh.h"
#include "model.h"

#include "textmesh.h"

#include "meshex.h"
#include "modelex.h"

#include "camera.h"

#include "renderable.h"

// Renderer
#include "lightdata.h"
#include "skybox.h"
#include "coloredskybox.h"
#include "sixsidedskybox.h"

#include "shader.h"
#include "softwareshader.h"
#include "unlitshader.h"
#include "phongshader.h"
#include "rendercontext.h"

// Assets
#include "texturefactory.h"
#include "objloader.h"

#include "setup.h"

using namespace AwesomeRenderer;

void AwesomeRenderer::SetupScene(RenderContext& mainContext, RenderContext& hudContext, ObjLoader& objLoader, TextureFactory& textureFactory)
{
	// Shader
	PhongShader* phongShader = new PhongShader();
	UnlitShader* unlitShader = new UnlitShader();

	if (FALSE)
	{
		Node* node = new Node();
		Model* model = new Model();

		Transformation* transform = new Transformation();

		node->AddComponent(model);
		node->AddComponent(transform);

		//transform->SetScale(Vector3(0.1f, 0.1f, 0.1f));
		//transform->SetScale(Vector3(0.2f, 0.2f, 0.2f));
		//objLoader.Load("../Assets/Town/town.obj", *model);
		objLoader.Load("../Assets/crytek-sponza/sponza.obj", *model);
		//objLoader.Load("../Assets/Castle01/castle.obj", *model);

		mainContext.nodes.push_back(node);
	}

	if (FALSE)
	{
		Node* node = new Node();

		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL | Mesh::VERTEX_TEXCOORD));
		mesh->AddQuad(Vector3(1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, 1.0f), Vector3(-1.0f, 0.0f, -1.0f));

		float uvScale = 5.0f;
		mesh->texcoords[0] = Vector2(1.0f, 1.0f) * uvScale;
		mesh->texcoords[1] = Vector2(0.0f, 1.0f) * uvScale;
		mesh->texcoords[2] = Vector2(0.0f, 0.0f) * uvScale;

		mesh->texcoords[3] = Vector2(0.0f, 0.0f) * uvScale;
		mesh->texcoords[4] = Vector2(1.0f, 0.0f) * uvScale;
		mesh->texcoords[5] = Vector2(1.0f, 1.0f) * uvScale;

		mesh->CalculateBounds();

		Texture* texture = NULL;
		textureFactory.GetAsset("../Assets/tiles.bmp", &texture);

		Sampler* sampler = new Sampler();
		sampler->texture = texture;

		PhongMaterial* material = new PhongMaterial(*(new Material()));
		material->provider.shader = phongShader;
		material->diffuseMap = sampler;
		material->specularColor = Color::WHITE * 0.1f;
		material->shininess = 1.0f;

		Model* model = new Model();
		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();

		node->AddComponent(model);

		Transformation* transform = new Transformation();
		transform->SetScale(Vector3(5.0f, 5.0f, 5.0f));
		node->AddComponent(transform);

		mainContext.nodes.push_back(node);
	}
}

void AwesomeRenderer::SetupLighting(LightData& lightData)
{
	lightData.numPixelLights = 8;
	lightData.ambient = Color::BLACK;

	{
		LightData::Light& light = lightData.lights[0];
		//*
		light.type = LightData::LightType::POINT;
		light.position = Vector3(0.0f, 0.9f, 0.5f);
		light.color = Color(0.78f, 0.78f, 0.78f);
		light.intensity = 1.0f;
		/*/
		light.type = LightData::LightType::DIRECTIONAL;
		light.direction = Vector3(-0.5f, -0.8f, -0.5f);
		light.color = Color::WHITE;
		light.direction.normalize();
		light.intensity = 2.0f;
		//*/

		light.enabled = true;
	}

	{
		LightData::Light& light = lightData.lights[1];
		light.position = Vector3(5.0f, 3.0f, 5.0f);
		light.type = LightData::LightType::POINT;
		light.color = Color::WHITE;// Color::BLUE;
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.1f;
		light.quadricAttenuation = 0.02f;
		light.intensity = 0.1f;
		light.enabled = false;
	}

	{
		LightData::Light& light = (lightData.lights[2] = lightData.lights[1]);
		light.position = Vector3(5.0f, 3.0f, -5.0f);
		light.color = Color::WHITE;//Color::RED;
	}

	{
		LightData::Light& light = (lightData.lights[3] = lightData.lights[1]);
		light.position = Vector3(-5.0f, 3.0f, -5.0f);
		light.color = Color::WHITE;//Color::PURPLE;
	}

	{
		LightData::Light& light = (lightData.lights[4] = lightData.lights[1]);
		light.position = Vector3(-5.0f, 3.0f, 5.0f);
		light.color = Color::WHITE;//Color::GREEN;
	}

	{
		LightData::Light& light = lightData.lights[5];
		light.position = Vector3(0.0f, 2.0f, 0.0f);
		light.type = LightData::LightType::POINT;
		light.color = Color(165, 250, 255);
		light.constantAttenuation = 0.0f;
		light.lineairAttenuation = 0.1f;
		light.quadricAttenuation = 0.02f;
		light.intensity = 0.05f;
		light.enabled = false;
	}

}

void AwesomeRenderer::SetupCornellBox(RenderContext& context, Camera& camera)
{
	const Vector3 cameraPosition = Vector3(0.0f, 0.5f, -1.5f);
	camera.SetLookAt(cameraPosition, cameraPosition + Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));
	
	const Color wallWhite = Color(0.725f, 0.71f, 0.68f);
	const Color wallSpecular = Color::WHITE * 0.2f;
	const float wallRoughness = 0.8f;

	/*
	const Color sphereDiffuse = Color::BLACK;
	const Color sphereSpecular(0.6f, 0.6f, 0.6f);
	const float sphereRoughness = 0.4f;
	const float sphereMetallic = 1;
	/*/
	const Color sphereDiffuse(0.5f, 0.5f, 0.5f);
	const Color sphereSpecular(0.1f, 0.1f, 0.1f);
	const float sphereRoughness = 0.4f;
	const float sphereMetallic = 0;
	//*/

	{
		// Left wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(-0.5f, 0.0f, 0.0f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = Color(0.63f, 0.0065f, 0.05f);
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Plane(0.0f, Vector3(1.0f, 0.0f, 0.0f));
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// Right wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.5f, 0.0f, 0.0f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = Color(0.14f, 0.45f, 0.091f);
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Plane(0.0f, Vector3(-1.0f, 0.0f, 0.0f));
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// Floor
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Plane(0.0f, Vector3(0.0f, 1.0f, 0.0f));
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// Ceiling
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 1.0f, 0.0f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Plane(0.0f, Vector3(0.0f, -1.0f, 0.0f));
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// Back wall
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 0.0f, 1.0f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0;
		material->roughness = wallRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Plane(0.0f, Vector3(0.0f, 0.0f, -1.0f));
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// Left sphere
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(-0.25f, 0.0f, 0.65f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = sphereDiffuse;
		material->specular = sphereSpecular;
		material->metallic = sphereMetallic;
		material->roughness = sphereRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Sphere(Vector3(0.0f, 0.15f, 0.0f), 0.15f);
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}

	{
		// right sphere
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.15f, 0.0f, 0.25f));
		node->AddComponent(transform);
		
		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = sphereDiffuse;
		material->specular = sphereSpecular;
		material->metallic = sphereMetallic;
		material->roughness = sphereRoughness;

		Renderable* renderable = new Renderable();
		renderable->shape = new Sphere(Vector3(0.0f, 0.18f, 0.0f), 0.18f);
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.nodes.push_back(node);
	}
}

void AwesomeRenderer::SetupSpheres(RenderContext& context, Camera& camera)
{
	const Vector3 cameraPosition = Vector3(3.2f, 1.8f, 7.5f);
	camera.SetLookAt(cameraPosition, cameraPosition - Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));

	const float MIN_ROUGHNESS = 0.1f;
	const float SPHERE_RADIUS = 0.6f;
	const float SPHERE_SPACING = 0.4f;
	const uint32_t SPHERES_PER_ROW = 5;
	const Vector3 origin(0.0f, 0.5f + SPHERE_RADIUS, 0.0f);

	for (uint32_t row = 0; row < 2; ++row)
	{
		for (uint32_t sphereIdx = 0; sphereIdx < SPHERES_PER_ROW; ++sphereIdx)
		{
			Node* node = new Node();

			Vector3 position = origin;
			position[0] += sphereIdx * (SPHERE_RADIUS * 2 + SPHERE_SPACING);
			position[1] += row * (SPHERE_RADIUS * 2 + SPHERE_SPACING);

			Transformation* transform = new Transformation();
			transform->SetPosition(position);
			node->AddComponent(transform);

			PbrMaterial* material = new PbrMaterial(*(new Material()));
			material->roughness = MIN_ROUGHNESS + (sphereIdx / (float)(SPHERES_PER_ROW - 1.0f)) * (1.0f - MIN_ROUGHNESS);

			if (row == 0)
			{
				material->albedo = Color::WHITE * 0.8f;
				material->specular = Color::WHITE * 0.2f;
				material->metallic = 0;
			}
			else if (row == 1)
			{
				material->albedo = Color::BLACK;
				material->specular = Color::WHITE * 0.9f;
				material->metallic = 1;
			}

			Renderable* renderable = new Renderable();
			renderable->shape = new Sphere(Vector3(0.0f, 0.0f, 0.0f), SPHERE_RADIUS);

			renderable->material = &material->provider;

			node->AddComponent(renderable);

			context.nodes.push_back(node);
		}
	}
}