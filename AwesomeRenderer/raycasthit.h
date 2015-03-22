#ifndef _RAY_CAST_HIT_H_
#define _RAY_CAST_HIT_H_

namespace AwesomeRenderer
{
	class Node;

	struct RaycastHit
	{
		Vector3 point;
		Vector3 normal;

		float distance;

		Vector3 barycentricCoords;

		const Node* node;

		RaycastHit() : distance(FLT_MAX), node(NULL)
		{

		}

	};

}

#endif