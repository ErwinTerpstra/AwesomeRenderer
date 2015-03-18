#ifndef _MESH_GL_H_
#define _MESH_GL_H_

#include "awesomerenderer.h"

#include "mesh.h"

namespace AwesomeRenderer
{
	class Mesh;

	class MeshGL : public Extension<Mesh>
	{
	public:
		GLuint vertexArray;
		GLuint vertexBuffers[Mesh::MAX_VERTEX_ATTRIBUTES];

		GLuint indexBuffer;

	public:

		MeshGL(Mesh& mesh);

		void CreateBuffers();

	private:
		void AddAttributeBuffer(uint32_t idx, GLuint* buffer);

	};

}

#endif	