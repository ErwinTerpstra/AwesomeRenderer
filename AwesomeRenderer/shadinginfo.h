#ifndef _SHADING_INFO_H_
#define _SHADING_INFO_H_

#include "awesomerenderer.h"
#include "raycasthit.h"

namespace AwesomeRenderer
{

	struct ShadingInfo
	{
		Color color;
		RaycastHit hitInfo;
	};

}

#endif