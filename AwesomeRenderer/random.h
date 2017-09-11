#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <cstdlib>
#include "debug.h"

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

		float NextFloat() 
		{
			float scale = ((float)RAND_MAX) + 1;
			float base = rand() / scale;
			float fine = rand() / scale;

			return base + fine / scale;
		}

		int NextInt(int min, int max) 
		{
			assert(max >= min);
			return min + (int) floor(NextFloat() * (max - min)); 
		}
	};

}

#endif