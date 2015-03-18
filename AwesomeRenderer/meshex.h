#ifndef _MESH_EX_H_
#define _MESH_EX_H_

#include "awesomerenderer.h"
#include "component.h"
#include "kdtree.h"

namespace AwesomeRenderer
{
	class Mesh;
	class Triangle3D;

	class MeshEx : public Extension<Mesh>
	{
	public:
		static const int ID;

		std::vector<Triangle3D*> triangles;

		KDTree tree;

	public:
		MeshEx(Mesh& mesh);
		~MeshEx();

		Triangle3D& AddTri(const Vector3& a, const Vector3& b, const Vector3& c);
		void OptimizeTree();
	};
}

#endif