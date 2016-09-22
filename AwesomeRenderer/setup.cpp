#include "stdafx.h"
#include "setup.h"

#include "awesomerenderer.h"
#include "context.h"

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

using namespace AwesomeRenderer;

Setup::Setup(Context& context) : context(context)
{

}

void Setup::SetupScene()
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
		context.objLoader->Load("../Assets/crytek-sponza/sponza.obj", *model);
		//objLoader.Load("../Assets/Castle01/castle.obj", *model);

		context.mainContext->nodes.push_back(node);
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
		context.textureFactory->GetAsset("../Assets/tiles.bmp", &texture);

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

		context.mainContext->nodes.push_back(node);
	}
}

void Setup::SetupLighting()
{
	context.mainContext->lightData->numPixelLights = 8;
	context.mainContext->lightData->ambient = Color::BLACK;
}

void Setup::SetupCornellBox()
{
	// CAMERA
	const Vector3 cameraPosition = Vector3(0.0f, 0.5f, -1.5f);
	context.mainCamera->SetLookAt(cameraPosition, cameraPosition + Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));
	
	// LIGHTING
	{
		LightData::Light& light = context.mainContext->lightData->lights[0];
		light.type = LightData::LightType::POINT;
		light.position = Vector3(0.0f, 0.9f, 0.5f);
		light.color = Color(0.78f, 0.78f, 0.78f);
		light.intensity = 1.0f;

		light.enabled = true;
	}

	// GEOMETRY
	const Color wallWhite = Color(0.725f, 0.71f, 0.68f);
	const Color wallSpecular = Color::WHITE * 0.2f;
	const float wallRoughness = 0.8f;

	//*
	const Color sphereDiffuse = Color::BLACK;
	const Color sphereSpecular(0.6f, 0.6f, 0.6f);
	const float sphereRoughness = 0.5f;
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

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
	}

	if (FALSE)
	{
		// Light
		Node* node = new Node();

		Transformation* transform = new Transformation();
		transform->SetPosition(Vector3(0.0f, 1.0f - 1e-5f, 0.0f));
		transform->SetScale(Vector3(0.5f, 1.0f, 0.5f));
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = Color::BLACK;
		material->specular = Color::BLACK;
		material->metallic = 0;
		material->roughness = wallRoughness;
		material->provider.emission = Color(0.78f, 0.78f, 0.78f);

		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL));
		mesh->AddQuad(Vector3(0.5f, 0.0f, -0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, -0.5f));
		mesh->CalculateBounds();

		Model* model = new Model();
		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();
		node->AddComponent(model);

		ModelEx* modelEx = new ModelEx(*model);		

		Renderable* renderable = new Renderable();
		renderable->shape = modelEx->meshes[0];
		renderable->material = &material->provider;

		node->AddComponent(renderable);

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
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

		context.mainContext->nodes.push_back(node);
	}

	if (FALSE)
	{
		Node* node = new Node();

		Quaternion q;
		cml::quaternion_rotation_axis_angle(q, Vector3(0.0f, 1.0f, 0.0f), (float) PI);

		Transformation* transform = new Transformation();
		transform->SetRotation(q);
		node->AddComponent(transform);

		PbrMaterial* material = new PbrMaterial(*(new Material()));
		material->albedo = wallWhite;
		material->specular = wallSpecular;
		material->metallic = 0.0f;
		material->roughness = wallRoughness;


		Model* model = new Model();
		/*
		context.objLoader->Load("../Assets/bunny_lowpoly.obj", *model);
		transform->SetPosition(Vector3(-0.08f, -0.14f, 0.5f));
		transform->SetScale(Vector3(4.0f, 4.0f, 4.0f));
		/*/
		transform->SetPosition(Vector3(0.0f, 0.05f, 0.5f));
		transform->SetScale(Vector3(0.2f, 0.2f, 0.2f));
		context.objLoader->Load("../Assets/bunny.obj", *model);
		//*/

		/*
		Mesh* mesh = new Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL));
		mesh->CreateCube(Vector3(0.0f, 0.25f, 0.0f), 0.25f);
		mesh->CalculateBounds();

		model->AddMesh(mesh, &material->provider);
		model->CalculateBounds();
		*/

		node->AddComponent(model);

		ModelEx* modelEx = new ModelEx(*model);

		for (uint32_t meshIdx = 0; meshIdx < modelEx->meshes.size(); ++meshIdx)
		{
			Node* meshNode = new Node();

			meshNode->AddComponent(transform);

			Renderable* renderable = new Renderable();
			renderable->shape = modelEx->meshes[0];
			renderable->material = &material->provider;

			meshNode->AddComponent(renderable);

			context.mainContext->nodes.push_back(meshNode);
		}

		context.mainContext->nodes.push_back(node);
	}
}

void Setup::SetupSpheres()
{
	const Vector3 cameraPosition = Vector3(3.2f, 1.8f, 7.5f);
	context.mainCamera->SetLookAt(cameraPosition, cameraPosition - Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0));

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

			context.mainContext->nodes.push_back(node);
		}
	}
}