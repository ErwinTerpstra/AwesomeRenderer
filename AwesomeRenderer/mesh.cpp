#include "awesomerenderer.h"

using namespace AwesomeRenderer;


Mesh::Mesh(VertexAttributes attributes) :
	vertices(0), colors(0), texcoords(0), normals(0), indices(0), attributes(attributes)
{
	
}

void Mesh::Reserve(int numTriangles, int numVertices)
{	
	if (HasAttribute(VERTEX_POSITION))
		vertices.reserve(vertices.size() + numVertices);

	if (HasAttribute(VERTEX_COLOR))
		colors.reserve(colors.size() + numVertices);

	if (HasAttribute(VERTEX_TEXCOORD))
		texcoords.reserve(texcoords.size() + numVertices);

	if (HasAttribute(VERTEX_NORMAL))
		normals.reserve(normals.size() + numVertices);
}

void Mesh::CalculateBounds()
{
	Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 max(FLT_MIN, FLT_MIN, FLT_MIN);

	std::vector<Vector3>::iterator it;

	// Find the axis aligned minimum and maximum planes for the vertices
	for (it = vertices.begin(); it != vertices.end(); ++it)
	{
		const Vector3& vertex = *it;

		min[0] = std::min(vertex[0], min[0]);
		min[1] = std::min(vertex[1], min[1]);
		min[2] = std::min(vertex[2], min[2]);
		
		max[0] = std::max(vertex[0], max[0]);
		max[1] = std::max(vertex[1], max[1]);
		max[2] = std::max(vertex[2], max[2]);
	}

	bounds.Initialize(min, max);
}
	
void Mesh::AddTri(const Vector3& a, const Vector3& b, const Vector3& c)
{
	vertices.push_back(a);
	vertices.push_back(b);
	vertices.push_back(c);

	if (a[1] > 0.0f)
	{
		texcoords.push_back(Vector2(1.0f, 1.0f));
		texcoords.push_back(Vector2(1.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
	}
	else
	{
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 1.0f));
		texcoords.push_back(Vector2(1.0f, 1.0f));
	}
	
	indices.push_back(vertices.size() - 3);
	indices.push_back(vertices.size() - 2);
	indices.push_back(vertices.size() - 1);
}

void Mesh::AddQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
{
	AddTri(c, b, a);
	AddTri(a, d, c);
}

void Mesh::CreateCube(const Vector3& center, float extents)
{
	Reserve(12, 36);

    Vector3 topFrontLeft        = center + (extents * Vector3(-1.0f,  1.0f, -1.0f));
    Vector3 topFrontRight       = center + (extents * Vector3( 1.0f,  1.0f, -1.0f));
    Vector3 topBackLeft         = center + (extents * Vector3(-1.0f,  1.0f,  1.0f));
    Vector3 topBackRight        = center + (extents * Vector3( 1.0f,  1.0f,  1.0f));
    Vector3 bottomFrontLeft     = center + (extents * Vector3(-1.0f, -1.0f, -1.0f));
    Vector3 bottomFrontRight    = center + (extents * Vector3( 1.0f, -1.0f, -1.0f));
    Vector3 bottomBackLeft      = center + (extents * Vector3(-1.0f, -1.0f,  1.0f));
    Vector3 bottomBackRight     = center + (extents * Vector3( 1.0f, -1.0f,  1.0f));

    AddQuad(topFrontLeft,		topBackLeft,		topBackRight,		topFrontRight);		// TOP
    AddQuad(bottomFrontRight,	bottomBackRight,	bottomBackLeft,		bottomFrontLeft);	// BOTTOM
    AddQuad(topFrontLeft,		bottomFrontLeft,	bottomBackLeft,		topBackLeft);		// LEFT
    AddQuad(topBackRight,		bottomBackRight,	bottomFrontRight,	topFrontRight);		// RIGHT
    AddQuad(topFrontRight,		bottomFrontRight,	bottomFrontLeft,	topFrontLeft);		// FRONT
    AddQuad(topBackLeft,		bottomBackLeft,		bottomBackRight,	topBackRight);		// BACK
}