#ifndef _COLORED_SKYBOX_H_
#define _COLORED_SKYBOX_H_

#include "awesomerenderer.h"

#include "skybox.h"

namespace AwesomeRenderer
{

	class ColoredSkybox : public Skybox
	{
	public:

		Color top;
		Color bottom;

	public:

		ColoredSkybox()
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