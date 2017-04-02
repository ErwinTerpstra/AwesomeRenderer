#ifndef _MESH_H_
#define _MESH_H_

#include "awesomerenderer.h"
#include "shape.h"
#include "component.h"
#include "aabb.h"

namespace AwesomeRenderer
{
	class Mesh : public ExtensionProvider<Mesh>
	{
	
	public:

		enum Extensions
		{
			MESH_GL,
			MESH_EX,
		};

		enum VertexAttributes
		{
			VERTEX_POSITION		= 1,
			VERTEX_COLOR		= 2,
			VERTEX_TEXCOORD		= 4,
			VERTEX_NORMAL		= 8,
			VERTEX_TANGENTS		= 16,
			VERTEX_BITANGENTS	= 32,

			MAX_VERTEX_ATTRIBUTES = 6,

			VERTEX_ALL			= 255
		};

	public:
		std::vector<Vector3> vertices;
		std::vector<Color> colors;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;
		std::vector<Vector3> tangents;
		std::vector<Vector3> bitangents;

		std::vector<uint32_t> indices;

		VertexAttributes attributes;
		
		AABB bounds;

	private:
		int cVertex, cIndex;

	public:
		Mesh(VertexAttributes attributes);
		
		void Clear();
		void Reserve(uint32_t numTriangles, uint32_t numVertices);

		void CreateCube(const Vector3& center, float extents);
		void CalculateBounds();
		void CalculateTangentBasis();

		__inline bool HasAttribute(VertexAttributes attribute) const { return (this->attributes & attribute) != 0; }

		void AddTri(const Vector3& a, const Vector3& b, const Vector3& c);
		void AddQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);

	};

}

#endif