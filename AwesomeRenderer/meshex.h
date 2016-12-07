#ifndef _MESH_EX_H_
#define _MESH_EX_H_

#include "awesomerenderer.h"
#include "component.h"
#include "kdtree.h"
#include "mesh.h"

namespace AwesomeRenderer
{
	class Triangle3D;

	class MeshEx : public Shape, public Extension<Mesh, MeshEx>
	{
	public:
		static const int ID;

		std::vector<Triangle3D*> triangles;

		KDTree tree;

		Matrix44 worldMtx;
		Matrix44 world2object;

	public:
		MeshEx(Mesh& mesh);
		~MeshEx();

		void OptimizeTree();

		void Transform(const Matrix44& mtx);

		bool IntersectRay(const Ray& ray, RaycastHit& hitInfo, float maxDistance) const;

		const Primitive& GetPrimitive() const;

		static uint32_t ExtensionID() { return Mesh::MESH_EX; }
	};
}

#endif