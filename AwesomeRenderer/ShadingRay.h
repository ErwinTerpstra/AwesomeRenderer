#ifndef _SHADING_RAY_H_
#define _SHADING_RAY_H_

#include "ray.h"

namespace AwesomeRenderer
{
	class ShadingRay : public Ray
	{

	public:
		float distance;
	};
}

#endif