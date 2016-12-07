#ifndef _RAY_CAST_HIT_H_
#define _RAY_CAST_HIT_H_

#include "treeelement.h"

namespace AwesomeRenderer
{
	struct RaycastHit
	{
		Vector3 point;
		Vector3 normal;

		float distance;

		Vector3 barycentricCoords;
		Vector2 uv;

		const TreeElement* element;

		RaycastHit() : distance(FLT_MAX), element(NULL)
		{

		}

		RaycastHit(const RaycastHit& other) :
			point(other.point), normal(other.normal), distance(other.distance), 
			barycentricCoords(other.barycentricCoords), uv(other.uv),
			element(other.element)
		{

		}

	};

}

#endif