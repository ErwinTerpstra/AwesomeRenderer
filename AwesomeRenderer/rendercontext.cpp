#include "stdafx.h"
#include "awesomerenderer.h"

#include "rendercontext.h"
#include "lightdata.h"

#include "node.h"
#include "transformation.h"
#include "model.h"
#include "mesh.h"
#include "renderable.h"
#include "kdtreenode.h"

using namespace AwesomeRenderer;

RenderContext::RenderContext() : 
	camera(NULL), renderTarget(NULL), lightData(NULL), skybox(NULL), 
	clearFlags(RenderTarget::BUFFER_ALL), tree(20)
{

}

void RenderContext::Optimize()
{
	Update();

	Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	KDTreeNode::ElementList& elements = tree.rootNode->GetElements();
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		Renderable* renderable = (*it)->GetComponent<Renderable>();

		if (renderable != NULL)
		{
			//*
			AABB bounds;
			renderable->GetPrimitive().CalculateBounds(bounds);

			Vector3 boundsMin = bounds.Min();
			min[0] = std::min(min[0], boundsMin[0]);
			min[1] = std::min(min[1], boundsMin[1]);
			min[2] = std::min(min[2], boundsMin[2]);

			Vector3 boundsMax = bounds.Max();
			max[0] = std::max(max[0], boundsMax[0]);
			max[1] = std::max(max[1], boundsMax[1]);
			max[2] = std::max(max[2], boundsMax[2]);
			//*/

			elements.push_back(renderable);
		}
	}
	
	/*
	min = Vector3(-5.0f, -5.0f, -5.0f);
	max = Vector3( 5.0f,  5.0f,  5.0f);
	//*/

	Vector3 epsilon(0.1f, 0.1f, 0.1f);
	min -= epsilon;
	max += epsilon;
	tree.Optimize(AABB(min, max));

	printf("[RenderContext]: Scene tree optimized, analyzing...\n");
	tree.Analyze();
}

void RenderContext::Update()
{
	// Prepare models in scene
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
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