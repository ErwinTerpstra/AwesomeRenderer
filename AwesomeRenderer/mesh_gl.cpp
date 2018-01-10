
#include "awesomerenderer.h"
#include "mesh_gl.h"
#include "util_gl.h"

using namespace AwesomeRenderer;

MeshGL::MeshGL(Mesh& mesh) : Extension(mesh)
{

}

void MeshGL::Allocate()
{
	assert(glGenVertexArrays != NULL && glBindVertexArray != NULL);

	// Create and bind Vertex Array Object
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	if (provider.HasAttribute(Mesh::VERTEX_POSITION))
	{
		AddAttributeBuffer(ATTR_POSITION, &vertexBuffers[ATTR_POSITION]);
		glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
	}

	if (provider.HasAttribute(Mesh::VERTEX_NORMAL))
	{
		AddAttributeBuffer(ATTR_NORMAL, &vertexBuffers[ATTR_NORMAL]);
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TEXCOORD))
	{
		AddAttributeBuffer(ATTR_TEXCOORD, &vertexBuffers[ATTR_TEXCOORD]);
		glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), NULL);
	}

	if (provider.HasAttribute(Mesh::VERTEX_COLOR))
	{
		AddAttributeBuffer(ATTR_COLOR, &vertexBuffers[ATTR_COLOR]);
		glVertexAttribPointer(ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Color), NULL);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TANGENTS))
	{
		AddAttributeBuffer(ATTR_TANGENT, &vertexBuffers[ATTR_TANGENT]);
		glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
	}

	if (provider.HasAttribute(Mesh::VERTEX_BITANGENTS))
	{
		AddAttributeBuffer(ATTR_BITANGENT, &vertexBuffers[ATTR_BITANGENT]);
		glVertexAttribPointer(ATTR_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
	}

	// Indices
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if (provider.indices.size() > 0)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, provider.indices.size() * sizeof(uint32_t), &provider.indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	GL_CHECK_ERROR(GL_NO_CALL);
}
void MeshGL::Apply()
{
	glBindVertexArray(vertexArray);

	if (provider.HasAttribute(Mesh::VERTEX_POSITION))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_POSITION]);
		glBufferData(GL_ARRAY_BUFFER, provider.vertices.size() * sizeof(GL_FLOAT) * 3, &provider.vertices[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_NORMAL))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, provider.normals.size() * sizeof(GL_FLOAT) * 3, &provider.normals[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TEXCOORD))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, provider.texcoords.size() * sizeof(GL_FLOAT) * 2, &provider.texcoords[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_COLOR))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_COLOR]);
		glBufferData(GL_ARRAY_BUFFER, provider.colors.size() * sizeof(GL_FLOAT) * 4, &provider.colors[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TANGENTS))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_TANGENT]);
		glBufferData(GL_ARRAY_BUFFER, provider.tangents.size() * sizeof(GL_FLOAT) * 3, &provider.tangents[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_BITANGENTS))
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ATTR_BITANGENT]);
		glBufferData(GL_ARRAY_BUFFER, provider.bitangents.size() * sizeof(GL_FLOAT) * 3, &provider.bitangents[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(0);

	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, provider.indices.size() * sizeof(uint32_t), &provider.indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GL_CHECK_ERROR(GL_NO_CALL);
}

void MeshGL::AddAttributeBuffer(uint32_t idx, GLuint* buffer)
{
	glGenBuffers(1, buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	// TODO: This should be called by the renderer. Attribute offsets that are not used should be disabled
	glEnableVertexAttribArray(idx);

	GL_CHECK_ERROR(GL_NO_CALL);
}