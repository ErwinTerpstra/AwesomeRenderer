#ifndef _MESH_GL_H_
#define _MESH_GL_H_

namespace AwesomeRenderer
{

	class MeshGL : public Extension<Mesh>
	{
	public:

		GLuint vertexArray;
		GLuint vertexBuffer;

		GLuint indexBuffer;

	public:

		MeshGL(Mesh& mesh);

		void CreateBuffers();
	};

}

#endif