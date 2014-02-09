#include "awesomerenderer.h"

using namespace AwesomeRenderer;

MeshEx::MeshEx(const Mesh& mesh)
{

	for (unsigned int cIndex = 0; cIndex < mesh.indices.size(); cIndex += 3)
	{
		// Retrieve vertex indices for this triangle
		int vIdx0 = mesh.indices[cIndex], vIdx1 = mesh.indices[cIndex + 1], vIdx2 = mesh.indices[cIndex + 2];

		// Create triangle object based on the vertex data
		Triangle3D& triangle = AddTri(mesh.vertices[vIdx0], mesh.vertices[vIdx1], mesh.vertices[vIdx2]);
	}
}

MeshEx::~MeshEx()
{
	triangles.clear();
}

Triangle3D& MeshEx::AddTri(const Vector3& a, const Vector3& b, const Vector3& c)
{
	Triangle3D triangle(a, b, c);
	triangles.push_back(triangle);

	return triangles[triangles.size() - 1];
}