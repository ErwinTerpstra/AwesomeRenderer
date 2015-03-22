#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Skybox
	{
	public:

		Color top;
		Color bottom;

	public:

		Skybox()
		{

		}

		void Sample(const Vector3& direction, Color& color)
		{
			float y = std::max<float>(direction[1], 0.0f);
			VectorUtil<4>::Interpolate(bottom, top, y, color);
		}

	};

}

#endif