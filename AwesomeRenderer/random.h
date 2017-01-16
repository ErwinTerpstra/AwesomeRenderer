#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <cstdlib>
#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Random
	{

	public:
		static Random instance;

	public:
		Random()
		{

		}

		float NextFloat() { return ((uint32_t)rand()) / (float)RAND_MAX; }
		int NextInt(int min, int max) 
		{
			assert(max >= min);
			return min + floor(NextFloat() * (1.0f - FLT_EPSILON) * (max - min)); 
		}
	};

}

#endif