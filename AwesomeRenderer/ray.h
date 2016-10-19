#ifndef _RAY_H_
#define _RAY_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Ray
	{
	public:
		enum Type
		{
			UNKOWN,
			HIT_TEST,
			REFLECTION,
			TRANSMISSION,
		};
		
		Vector3 origin, direction;

		Type type;
	public:
		Ray();
		Ray(const Vector3& origin, const Vector3& direction);

	};

}

#endif