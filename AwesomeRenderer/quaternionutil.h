#ifndef _QUATERNION_UTIL_H_
#define _QUATERNION_UTIL_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class QuaternionUtil
	{
	public:
		static Quaternion AngleAxis(float angle, const Vector3& axis)
		{
			Quaternion result;
			cml::quaternion_rotation_axis_angle(result, axis, angle);

			return result;
		}
	};
}

#endif
