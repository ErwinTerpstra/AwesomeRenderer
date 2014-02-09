#include "awesomerenderer.h"

using namespace AwesomeRenderer;

RayTracer::RayTracer() : Renderer()
{

}

void RayTracer::Render()
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;

	// Prepare models in scene
	std::vector<Node*>::iterator it;
	
	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		Node& node = **it;
				
		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model.meshes.size(); ++cMesh)
		{
			// Transform bounding shape of mesh according to world transformation
			Mesh& mesh = *node.model.meshes[cMesh];
			mesh.bounds.Transform(node.transform.WorldMtx());
		}
	}

	// Iterate through all pixels in the output buffer
	for (int y = 0; y < frameBuffer->height; ++y)
	{
		for (int x = 0; x < frameBuffer->width; ++x)
		{
			// Create a ray from the camera near plane through this pixel
			Ray primaryRay;
			renderContext->camera->ViewportToRay(Vector2((float) x, (float) y), primaryRay);
			
			Trace(primaryRay, Point2(x, y));
		}

	}
}

void RayTracer::Trace(const Ray& ray, const Point2& screenPosition)
{
	Buffer* frameBuffer = renderContext->renderTarget->frameBuffer;
	Buffer* depthBuffer = renderContext->renderTarget->depthBuffer;
	float cameraDepth = renderContext->camera->farPlane - renderContext->camera->nearPlane;

	std::vector<Node*>::iterator it;

	for (it = renderContext->nodes.begin(); it != renderContext->nodes.end(); ++it)
	{
		Node& node = **it;
				
		// Iterate through submeshes in a node
		for (unsigned int cMesh = 0; cMesh < node.model.meshes.size(); ++cMesh)
		{
			Mesh& mesh = *node.model.meshes[cMesh];
			Material& material = *node.model.materials[cMesh];

			// Check if some part of the mesh is hit by the ray
			RaycastHit boundsHitInfo(ray);
			if (!mesh.bounds.IntersectRay(ray, boundsHitInfo))
				continue;

			shader.material = &material;

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


			}
		}
	}

	// Interpolate pixel data
	Shader::VertexToPixel interpolated;
	interpolated.screenPosition = Vector4(((float)screenPosition[0]) / frameBuffer->width,
										  ((float)screenPosition[1]) / frameBuffer->height,
										  hitInfo.distance / cameraDepth, 1.0f);
	interpolated.worldPosition = Vector4(hitInfo.point, 1.0f);


	if (depthBuffer != NULL)
	{
		// Depth testing
		float depth = 1.0f - interpolated.screenPosition[2];

		// Write to depth buffer
		depthBuffer->SetPixel(screenPosition[0], screenPosition[1], depth);
	}

	//VectorUtil<4>::Interpolate(mesh.colors[vIdx0], mesh.colors[vIdx1], mesh.colors[vIdx2], hitInfo.barycentricCoords, interpolated.color);
	VectorUtil<3>::Interpolate(mesh.normals[vIdx0], mesh.normals[vIdx1], mesh.normals[vIdx2], hitInfo.barycentricCoords, interpolated.normal);
	VectorUtil<2>::Interpolate(mesh.texcoords[vIdx0], mesh.texcoords[vIdx1], mesh.texcoords[vIdx2], hitInfo.barycentricCoords, interpolated.uv);

	// Compute pixel shading
	Shader::PixelInfo pixelInfo;
	shader.ProcessPixel(interpolated, pixelInfo);

	// Write to color buffer
	frameBuffer->SetPixel(screenPosition[0], screenPosition[1], pixelInfo.color);
}