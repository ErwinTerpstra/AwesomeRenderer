#include "awesomerenderer.h"

using namespace AwesomeRenderer;

MeshGL::MeshGL(Mesh& mesh) : Extension(mesh)
{

}

void MeshGL::CreateBuffers()
{
	// Create and bind Vertex Array Object
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	uint32_t idx = 0;

	if (base.HasAttribute(Mesh::VERTEX_POSITION) && base.vertices.size() > 0)
	{
		AddAttributeBuffer(idx, &vertexBuffers[idx]);
		glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, base.vertices.size() * sizeof(GL_FLOAT) * 3, &base.vertices[0], GL_STATIC_DRAW);

		++idx;
	}

	if (base.HasAttribute(Mesh::VERTEX_NORMAL) && base.normals.size() > 0)
	{
		AddAttributeBuffer(idx, &vertexBuffers[idx]);
		glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), NULL);
		glBufferData(GL_ARRAY_BUFFER, base.normals.size() * sizeof(GL_FLOAT)* 3, &base.normals[0], GL_STATIC_DRAW);

		++idx;
	}

	if (base.HasAttribute(Mesh::VERTEX_TEXCOORD) && base.texcoords.size() > 0)
	{
		AddAttributeBuffer(idx, &vertexBuffers[idx]);
		glVertexAttribPointer(idx, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), NULL);
		glBufferData(GL_ARRAY_BUFFER, base.texcoords.size() * sizeof(GL_FLOAT)* 2, &base.texcoords[0], GL_STATIC_DRAW);

		++idx;
	}

	if (base.HasAttribute(Mesh::VERTEX_COLOR) && base.colors.size() > 0)
	{
		AddAttributeBuffer(idx, &vertexBuffers[idx]);
		glVertexAttribPointer(idx, 4, GL_FLOAT, GL_FALSE, sizeof(Color), NULL);
		glBufferData(GL_ARRAY_BUFFER, base.colors.size() * sizeof(GL_FLOAT)* 4, &base.colors[0], GL_STATIC_DRAW);

		++idx;
	}

	// Indices
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	if (base.indices.size() > 0)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, base.indices.size() * sizeof(uint32_t), &base.indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);
}

void MeshGL::AddAttributeBuffer(uint32_t idx, GLuint* buffer)
{
	glGenBuffers(1, buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	glEnableVertexAttribArray(idx);
}