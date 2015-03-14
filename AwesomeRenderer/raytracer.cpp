#include "awesomerenderer.h"

using namespace AwesomeRenderer;

RayTracer::RayTracer() : Renderer()
{

}

void RayTracer::Initialize()
{

}

void RayTracer::Render()
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;

	// Iterate through all pixels in the output buffer
	for (uint32_t y = 0; y < frameBuffer->height; ++y)
	{
		for (uint32_t x = 0; x < frameBuffer->width; ++x)
		{
			// Create a ray from the camera near plane through this pixel
			Ray primaryRay;
			renderContext->camera->ViewportToRay(Vector2((float) x, (float) y), primaryRay);
			
			Trace(primaryRay, Point2(x, y));
		}

	}
}

void RayTracer::Present(Window& window)
{
	GdiBuffer* buffer = static_cast<GdiBuffer*>(renderContext->renderTarget->frameBuffer);

	if (buffer != NULL)
		window.DrawBuffer(*buffer);
}

void RayTracer::Cleanup()
{

}

void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::const_iterator it;

	// Iterate through all nodes in the scene
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node& node = **it;
		const Shape& shape = node.GetShape();

		// Perform the ray-triangle intersection
		RaycastHit hitInfo(ray);
		if (!shape.IntersectRay(ray, hitInfo))
			continue;

		// Only hits outside viewing frustum
		if (hitInfo.distance > cameraDepth)
			continue;

		if (depthBuffer != NULL)
		{
			// Depth testing
			float depth = 1.0f - hitInfo.distance / cameraDepth;

			if (depthBuffer->GetPixel(screenPosition[0], screenPosition[1]) >= depth)
				continue;

			// Write to depth buffer
			depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
		}

		// Write to color buffer
		frameBuffer->SetPixel(screenPosition[0], screenPosition[1], Color::WHITE);
	}
}

#if FALSE
void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::const_iterator it;

	// Iterate through all nodes in the scene
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node& node = **it;

		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model->meshes.size(); ++cMesh)
		{
			const Mesh& mesh = *node.model->meshes[cMesh];
			const Material& material = *node.model->materials[cMesh];

			// Check if some part of the mesh is hit by the ray
			RaycastHit boundsHitInfo(ray);
			if (!mesh.bounds.IntersectRay(ray, boundsHitInfo))
				continue;

			// Create an inversed transformation matrix to transform to object space
			Matrix44 world2object = node.transform->WorldMtx();
			world2object.inverse();

			// Transform ray to object space to use for intersection
			Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));
			objectSpaceRay.direction.normalize();

			// Stack for nodes we still have to traverse
			std::stack<const KDTree*> nodesLeft;

			// Start with the root node of the mesh
			nodesLeft.push(&mesh.As<MeshEx>()->tree);

			while (!nodesLeft.empty())
			{
				// Get the top node from the stack
				const KDTree* tree = nodesLeft.top();
				nodesLeft.pop();
				
				// The current node is a leaf node, this means we can check its contents
				if (tree->IsLeaf())
				{
					std::vector<const Object*>::const_iterator objectIt;

					for (objectIt = tree->objects.begin(); objectIt != tree->objects.end(); ++objectIt)
					{
						const Shape& shape = (*objectIt)->GetShape();

						// Perform the ray-triangle intersection
						RaycastHit hitInfo(objectSpaceRay);
						if (!shape.IntersectRay(objectSpaceRay, hitInfo))
							continue;

						// Only hits outside viewing frustum
						if (hitInfo.distance > cameraDepth)
							continue;

						if (depthBuffer != NULL)
						{
							// Depth testing
							float depth = 1.0f - hitInfo.distance / cameraDepth;

							if (depthBuffer->GetPixel(screenPosition[0], screenPosition[1]) >= depth)
								continue;

							// Write to depth buffer
							depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
						}

						// Write to color buffer
						frameBuffer->SetPixel(screenPosition[0], screenPosition[1], Color::WHITE);
					}
				}
				else
				{
					// Construct the plane along which this tree node is split
					Vector3 planeNormal(0.0f, 0.0f, 0.0f); planeNormal[tree->Axis()] = 1.0f;
					Plane splitPlane(tree->SplitPoint(), planeNormal);

					// Determine which side of the plane the origin of the ray is, this side should always be visited
					int side = splitPlane.SideOfPlane(objectSpaceRay.origin);
					const KDTree *near, *far;
					
					if (side > 0)
					{
						near = tree->upperNode;
						far = tree->lowerNode;
					}
					else
					{
						near = tree->lowerNode;
						far = tree->upperNode;
					}

					// If the ray intersects the split plane, we need to visit the far node
					RaycastHit hitInfo(objectSpaceRay);
					if (splitPlane.IntersectRay(objectSpaceRay, hitInfo))
						nodesLeft.push(far);
					
					// Push the near node last so that we visit it first
					nodesLeft.push(near);
				}
			}

		}
	}
}
#endif

#if FALSE
void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::const_iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		const Node& node = **it;
				
		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model.meshes.size(); ++cMesh)
		{
			const Mesh& mesh = *node.model.meshes[cMesh];
			const Material& material = *node.model.materials[cMesh];

			// Check if some part of the mesh is hit by the ray
			RaycastHit boundsHitInfo(ray);
			if (!mesh.bounds.IntersectRay(ray, boundsHitInfo))
				continue;

			for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
			{
				// Retrieve vertex indices for this triangle
				int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

				// Create triangle object based on the vertex data
				Triangle3D triangle(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);
				
				// Transform the triangle from model to world space
				triangle.Transform(node.transform.WorldMtx());
				triangle.PreCalculateBarycentric();

				// Perform the ray-triangle intersection
				RaycastHit hitInfo(ray);
				if (!triangle.IntersectRay(ray, hitInfo))
					continue; 

				// Only hits outside viewing frustum
				if (hitInfo.distance > cameraDepth)
					continue;
				
				// Interpolate pixel data
				SoftwareShader::VertexToPixel interpolated;
				interpolated.screenPosition = Vector4(((float)screenPosition[0]) / frameBuffer->width,
													  ((float)screenPosition[1]) / frameBuffer->height,
														hitInfo.distance / cameraDepth, 1.0f);
				interpolated.worldPosition = Vector4(hitInfo.point, 1.0f);


				if (depthBuffer != NULL)
				{
					// Depth testing
					float depth = 1.0f - interpolated.screenPosition[2];

					if (depthBuffer->GetPixel(screenPosition[0], screenPosition[1]) >= depth)
						continue;

					// Write to depth buffer
					depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
				}

				// Write to color buffer
				frameBuffer->SetPixel(screenPosition[0], screenPosition[1], Color::WHITE);
			}
		}
	}

}
#endif