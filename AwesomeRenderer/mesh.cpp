
#include "awesomerenderer.h"
#include "mesh.h"

using namespace AwesomeRenderer;

Mesh::Mesh(VertexAttributes attributes) :
vertices(0), colors(0), texcoords(0), normals(0), tangents(0), bitangents(0), indices(0), attributes(attributes), bounds()
{
	
}

void Mesh::Reserve(uint32_t numTriangles, uint32_t numVertices)
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

void Mesh::Clear()
{
	vertices.clear();
	colors.clear();
	texcoords.clear();
	normals.clear();
	tangents.clear();
	bitangents.clear();

	indices.clear();
}

void Mesh::CalculateBounds()
{
	Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

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

void Mesh::CalculateTangentBasis()
{
	attributes = (VertexAttributes) (attributes | VERTEX_TANGENTS | VERTEX_BITANGENTS);

	// Keep track of how many faces make use of each vertex
	uint32_t* faceCounts = new uint32_t[vertices.size()];

	tangents.resize(vertices.size());
	bitangents.resize(vertices.size());

	// Initialize the lists to zero
	for (uint32_t vertexIdx = 0; vertexIdx < vertices.size(); ++vertexIdx)
	{
		faceCounts[vertexIdx] = 0;

		tangents[vertexIdx] = Vector3(0.0f, 0.0f, 0.0f);
		bitangents[vertexIdx] = Vector3(0.0f, 0.0f, 0.0f);
	}

	for (uint32_t triangleIdx = 0; triangleIdx < indices.size(); triangleIdx += 3)
	{
		uint32_t i0 = indices[triangleIdx + 0];
		uint32_t i1 = indices[triangleIdx + 1];
		uint32_t i2 = indices[triangleIdx + 2];

		const Vector3& v0 = vertices[i0];
		const Vector3& v1 = vertices[i1];
		const Vector3& v2 = vertices[i2];

		const Vector2& uv0 = texcoords[i0];
		const Vector2& uv1 = texcoords[i1];
		const Vector2& uv2 = texcoords[i2];

		Vector3 edge1 = v1 - v0;
		Vector3 edge2 = v2 - v0;

		Vector2 deltaUV1 = uv1 - uv0;
		Vector2 deltaUV2 = uv2 - uv0;
		
		float denom = (deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0]);

		Vector3 tangent, bitangent;
		if (fabs(denom) > 1e-5f)
		{
			float r = 1.0f / denom;

			tangent = VectorUtil<3>::Normalize((edge1 * deltaUV2[1] - edge2 * deltaUV1[1]) * r);
			bitangent = VectorUtil<3>::Normalize((edge2 * deltaUV1[0] - edge1 * deltaUV2[0]) * r);

			assert(VectorUtil<3>::IsNormalized(tangent));
			assert(VectorUtil<3>::IsNormalized(bitangent));
			//assert(fabs(VectorUtil<3>::Dot(tangent, bitangent)) < 1e-3f);
		}
		else
		{
			tangent = Vector3(1, 0, 0);
			bitangent = Vector3(0, 1, 0);
		}

		// Save the tangent and bitangent for each vertex in this face
		tangents[i0] += tangent;
		tangents[i1] += tangent;
		tangents[i2] += tangent;

		bitangents[i0] += bitangent;
		bitangents[i1] += bitangent;
		bitangents[i2] += bitangent;

		++faceCounts[i0];
		++faceCounts[i1];
		++faceCounts[i2];
	}

	// Average vectors over all faces that share the same vertex
	for (uint32_t vertexIdx = 0; vertexIdx < vertices.size(); ++vertexIdx)
	{
		tangents[vertexIdx] = VectorUtil<3>::Normalize(tangents[vertexIdx] / faceCounts[vertexIdx]);
		bitangents[vertexIdx] = VectorUtil<3>::Normalize(bitangents[vertexIdx] / faceCounts[vertexIdx]);
	}


	delete[] faceCounts;
}
	
void Mesh::AddTri(const Vector3& a, const Vector3& b, const Vector3& c)
{
	if (this->HasAttribute(VERTEX_POSITION))
	{
		vertices.push_back(a);
		vertices.push_back(b);
		vertices.push_back(c);
	}

	if (this->HasAttribute(VERTEX_COLOR))
	{
		colors.push_back(Color::WHITE);
		colors.push_back(Color::WHITE);
		colors.push_back(Color::WHITE);
	}

	if (this->HasAttribute(VERTEX_TEXCOORD))
	{
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
		texcoords.push_back(Vector2(0.0f, 0.0f));
	}

	if (this->HasAttribute(VERTEX_NORMAL))
	{
		Vector3 ab = b - a;
		Vector3 ac = c - a;
		Vector3 normal = cml::cross(ab, ac);
		normal.normalize();

		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
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