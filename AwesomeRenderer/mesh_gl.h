#ifndef _MESH_GL_H_
#define _MESH_GL_H_

#include "awesomerenderer.h"

#include "mesh.h"

namespace AwesomeRenderer
{
	class Mesh;

	class MeshGL : public Extension<Mesh, MeshGL>
	{
	public:
		enum VertexAttributeIndex
		{
			ATTR_POSITION		= 0,
			ATTR_NORMAL			= 1,
			ATTR_TEXCOORD		= 2,
			ATTR_COLOR			= 3,
			ATTR_TANGENT		= 4,
			ATTR_BITANGENT		= 5,
		};

		GLuint vertexArray;
		GLuint vertexBuffers[Mesh::MAX_VERTEX_ATTRIBUTES];

		GLuint indexBuffer;

	public:

		MeshGL(Mesh& mesh);

		void Allocate();
		void Apply();

		static uint32_t ExtensionID() { return Mesh::MESH_GL; }

	private:
		void AddAttributeBuffer(uint32_t idx, GLuint* buffer);

	};

}

#endif	