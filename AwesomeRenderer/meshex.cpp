#include "stdafx.h"
#include "awesomerenderer.h"
#include "meshex.h"
#include "mesh.h"
#include "triangle3d.h"

using namespace AwesomeRenderer;

MeshEx::MeshEx(Mesh& mesh) : Extension(mesh), tree(NULL), worldMtx()
{

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

		// Create triangle object based on the vertex data
		Triangle3D& triangle = AddTri(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);
	}

	tree.objects.insert(tree.objects.end(), triangles.begin(), triangles.end());
}

MeshEx::~MeshEx()
{
	triangles.clear();
}

Triangle3D& MeshEx::AddTri(const Vector3& a, const Vector3& b, const Vector3& c)
{
	Triangle3D* triangle = new Triangle3D(a, b, c);
	triangles.push_back(triangle);

	return *triangle;
}

void MeshEx::OptimizeTree()
{
	tree.Optimize(provider.bounds);
}

void MeshEx::Transform(const Matrix44& mtx)
{
	worldMtx = mtx;
}

bool MeshEx::IntersectRay(const Ray& ray, RaycastHit& hitInfo) const
{
	// Check if some part of the mesh is hit by the ray
	RaycastHit boundsHitInfo;
	if (!provider.bounds.IntersectRay(ray, boundsHitInfo))
		return false;

	// Create an inversed transformation matrix to transform to object space
	Matrix44 world2object = worldMtx;
	world2object.inverse();

	// Transform ray to object space to use for intersection
	Ray objectSpaceRay(cml::transform_point(world2object, ray.origin), cml::transform_vector(world2object, ray.direction));
	objectSpaceRay.direction.normalize();

	// Stack for nodes we still have to traverse
	std::stack<const KDTree*> nodesLeft;

	// Start with the root node of the mesh
	nodesLeft.push(&tree);

	float closestDistance = FLT_MAX;
	bool hit = false;

	while (!nodesLeft.empty())
	{
		// Get the top node from the stack
		const KDTree* tree = nodesLeft.top();
		nodesLeft.pop();

		// The current node is a leaf node, this means we can check its contents
		if (tree->IsLeaf())
		{
			std::vector<const Shape*>::const_iterator objectIt;

			for (objectIt = tree->objects.begin(); objectIt != tree->objects.end(); ++objectIt)
			{
				const Primitive& shape = (*objectIt)->GetShape();

				// Perform the ray-triangle intersection
				RaycastHit shapeHitInfo;
				if (!shape.IntersectRay(objectSpaceRay, shapeHitInfo))
					continue;

				// Only hits outside viewing frustum
				//if (hitInfo.distance > cameraDepth)
					//continue;
				
				if (shapeHitInfo.distance > closestDistance)
					continue;

				closestDistance = shapeHitInfo.distance;
				hitInfo = shapeHitInfo;
				hit = true;
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
			RaycastHit planeHitInfo;
			if (splitPlane.IntersectRay(objectSpaceRay, planeHitInfo))
				nodesLeft.push(far);

			// Push the near node last so that we visit it first
			nodesLeft.push(near);
		}
	}

	return hit;
}

const Primitive& MeshEx::GetShape() const
{
	return provider.bounds;
}