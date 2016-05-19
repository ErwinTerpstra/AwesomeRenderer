#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Skybox
	{
	public:
		
	public:

		Skybox()
		{

		}

		virtual void Sample(const Vector3& direction, Color& color) = 0;

	};

}

#endif