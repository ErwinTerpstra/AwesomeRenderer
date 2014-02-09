#ifndef _RAY_CAST_HIT_H_
#define _RAY_CAST_HIT_H_

namespace AwesomeRenderer
{
	struct RaycastHit
	{
		const Ray& ray;

		Vector3 point;
		float distance;

		Vector3 barycentricCoords;

		RaycastHit(const Ray& ray) : ray(ray)
		{

		}
	};

}

#endif