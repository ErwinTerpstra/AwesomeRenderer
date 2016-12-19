#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <cstdlib>

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
	};

}

#endif