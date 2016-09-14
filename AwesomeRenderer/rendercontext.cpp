#include "stdafx.h"
#include "awesomerenderer.h"

#include "rendercontext.h"
#include "lightdata.h"

#include "node.h"
#include "transformation.h"
#include "model.h"
#include "mesh.h"
#include "renderable.h"

using namespace AwesomeRenderer;

RenderContext::RenderContext() : camera(NULL), renderTarget(NULL), lightData(NULL), skybox(NULL), clearFlags(RenderTarget::BUFFER_ALL)
{

}

void RenderContext::Update()
{
	// Prepare models in scene
	std::vector<Node*>::iterator it;

	for (it = nodes.begin(); it != nodes.end(); ++it)
	{
		Node& node = **it;
		Transformation* transform = node.GetComponent<Transformation>();

		if (transform == NULL)
			continue;

		// Update global transformation matrix
		transform->CalculateMtx();

		// Update model
		Model* model = node.GetComponent<Model>();

		if (model != NULL)
			model->TransformBounds(transform->WorldMtx());

		// Update renderable object
		Renderable* renderable = node.GetComponent<Renderable>();

		if (renderable != NULL)
		{
			if (renderable->shape != NULL)
				renderable->shape->Transform(transform->WorldMtx());
		}
	}
}