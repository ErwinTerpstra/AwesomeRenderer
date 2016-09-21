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

		bool inside;

		const TreeElement* element;

		RaycastHit() : distance(FLT_MAX), element(NULL), inside(FALSE)
		{

		}

		RaycastHit(const RaycastHit& other) :
			point(other.point), normal(other.normal), distance(other.distance), barycentricCoords(other.barycentricCoords),
			inside(other.inside), element(other.element)
		{

		}

	};

}

#endif