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

	// Vertices
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, base.vertices.size() * sizeof(GL_FLOAT), &base.vertices[0], GL_STATIC_DRAW);
	
	// Indices
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, base.indices.size() * sizeof(uint32_t), &base.indices[0], GL_STATIC_DRAW);

}