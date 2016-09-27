#ifndef _SIX_SIDED_SKYBOX_H_
#define _SIX_SIDED_SKYBOX_H_

#include "awesomerenderer.h"

#include "skybox.h"
#include "sampler.h"

namespace AwesomeRenderer
{
	class SixSidedSkybox : public Skybox
	{
	public:

		Sampler* top;
		Sampler* bottom;
		Sampler* left;
		Sampler* right;
		Sampler* front;
		Sampler* back;

	public:

		SixSidedSkybox() : top(NULL), bottom(NULL), left(NULL), right(NULL), front(NULL), back(NULL)
		{

		}

		void Sample(const Vector3& direction, Color& color)
		{
			Sampler* side = NULL;
			Vector2 uv;
			Vector3 n;
			
			if (abs(direction[0]) > abs(direction[1]))
			{
				if (abs(direction[0]) > abs(direction[2]))
				{
					if (direction[0] > 0)
					{
						side = right;
						uv[0] = direction[2];
						uv[1] = direction[1];
					}
					else
					{
						side = left;
						uv[0] = -direction[2];
						uv[1] = direction[1];
					}

					n = Vector3((float) Util::Sign(direction[0]), 0, 0);
				}
				else
				{
					if (direction[2] > 0)
					{
						side = front;
						uv[0] = -direction[0];
						uv[1] = direction[1];
					}
					else
					{
						side = back;
						uv[0] = direction[0];
						uv[1] = direction[1];
					}
						
					n = Vector3(0, 0, (float) Util::Sign(direction[2]));
				}
			}
			else
			{
				if (abs(direction[1]) > abs(direction[2]))
				{
					if (direction[1] > 0)
					{
						side = top;
						uv[0] = direction[0];
						uv[1] = direction[2];
					}
					else
					{
						side = bottom;
						uv[0] = -direction[0];
						uv[1] = direction[2];
					}

					n = Vector3(0, (float) Util::Sign(direction[1]), 0);
				}
				else
				{
					if (direction[2] > 0)
					{
						side = front;
						uv[0] = -direction[0];
						uv[1] = direction[1];
					}
					else
					{
						side = back;
						uv[0] = direction[0];
						uv[1] = direction[1];
					}

					n = Vector3(0, 0, (float) Util::Sign(direction[2]));
				}
			}

			uv /= cml::dot(direction, n);

			uv[0] = (uv[0] + 1.0f) * 0.5f;
			uv[1] = (uv[1] + 1.0f) * 0.5f;
			
			side->Sample(uv, color);
		}

	};

}

#endif