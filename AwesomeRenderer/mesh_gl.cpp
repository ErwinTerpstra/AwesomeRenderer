#include "stdafx.h"
#include "awesomerenderer.h"
#include "mesh_gl.h"
#include "util_gl.h"

using namespace AwesomeRenderer;

MeshGL::MeshGL(Mesh& mesh) : Extension(mesh)
{

}

void MeshGL::CreateBuffers()
{
	if (glGenVertexArrays == NULL || glBindVertexArray == NULL)
	{
		printf("[MeshGL]: No support for vertex arrays!\n");
		return;
	}

	// Create and bind Vertex Array Object
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	
	if (provider.HasAttribute(Mesh::VERTEX_POSITION) && provider.vertices.size() > 0)
	{
		AddAttributeBuffer(ATTR_POSITION, &vertexBuffers[ATTR_POSITION]);
		glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.vertices.size() * sizeof(GL_FLOAT) * 3, &provider.vertices[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_NORMAL) && provider.normals.size() > 0)
	{
		AddAttributeBuffer(ATTR_NORMAL, &vertexBuffers[ATTR_NORMAL]);
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.normals.size() * sizeof(GL_FLOAT) * 3, &provider.normals[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TEXCOORD) && provider.texcoords.size() > 0)
	{
		AddAttributeBuffer(ATTR_TEXCOORD, &vertexBuffers[ATTR_TEXCOORD]);
		glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.texcoords.size() * sizeof(GL_FLOAT) * 2, &provider.texcoords[0], GL_STATIC_DRAW);
	}
	
	if (provider.HasAttribute(Mesh::VERTEX_COLOR) && provider.colors.size() > 0)
	{
		AddAttributeBuffer(ATTR_COLOR, &vertexBuffers[ATTR_COLOR]);
		glVertexAttribPointer(ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Color), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.colors.size() * sizeof(GL_FLOAT) * 4, &provider.colors[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_TANGENTS) && provider.tangents.size() > 0)
	{
		AddAttributeBuffer(ATTR_TANGENT, &vertexBuffers[ATTR_TANGENT]);
		glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.tangents.size() * sizeof(GL_FLOAT) * 3, &provider.tangents[0], GL_STATIC_DRAW);
	}

	if (provider.HasAttribute(Mesh::VERTEX_BITANGENTS) && provider.bitangents.size() > 0)
	{
		AddAttributeBuffer(ATTR_BITANGENT, &vertexBuffers[ATTR_BITANGENT]);
		glVertexAttribPointer(ATTR_BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, provider.bitangents.size() * sizeof(GL_FLOAT) * 3, &provider.bitangents[0], GL_STATIC_DRAW);
	}

	// Indices
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if (provider.indices.size() > 0)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, provider.indices.size() * sizeof(uint32_t), &provider.indices[0], GL_STATIC_DRAW);
	
	glBindVertexArray(0);

	GL_CHECK_ERROR(GL_NO_CALL);
}

void MeshGL::AddAttributeBuffer(uint32_t idx, GLuint* buffer)
{
	glGenBuffers(1, buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	glEnableVertexAttribArray(idx);

	GL_CHECK_ERROR(GL_NO_CALL);
}