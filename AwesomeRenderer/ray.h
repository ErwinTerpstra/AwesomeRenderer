#ifndef _RAY_H_
#define _RAY_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Ray
	{
	public:

		Vector3 origin, direction;
	public:
		Ray();
		Ray(const Vector3& origin, const Vector3& direction);

	};

}

#endif