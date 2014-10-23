#ifndef _MESH_H_
#define _MESH_H_

namespace AwesomeRenderer
{
	class Mesh : public Object, public Extendee<Mesh>
	{
	
	public:
		enum VertexAttributes
		{
			VERTEX_POSITION		= 1,
			VERTEX_COLOR		= 2,
			VERTEX_TEXCOORD		= 4,
			VERTEX_NORMAL		= 8,

			MAX_VERTEX_ATTRIBUTES = 4,

			VERTEX_ALL			= 255
		};

	public:
		std::vector<Vector3> vertices;
		std::vector<Color> colors;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;

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

		__inline bool HasAttribute(VertexAttributes attribute) const { return (this->attributes & attribute) != 0; }

		void AddTri(const Vector3& a, const Vector3& b, const Vector3& c);
		void AddQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d);

		virtual const Shape& GetShape() const { return bounds; }

	};

}

#endif