#ifndef _TIMER_H_
#define _TIMER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	
	struct TimingInfo
	{
		float elapsedSeconds;
		float totalSeconds;

		int totalFrames;
		int tickCount;
	};

	class Timer
	{
	public:

	private:

		TimingInfo lastInfo;

		float minFrameTime, maxFrameTime;

	public:
		Timer(float minFrameTime = 0.000001f, float maxFrameTime = 1.0f);

		const TimingInfo& Tick();

		float Poll();

	};

}

#endif