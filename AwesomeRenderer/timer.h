#ifndef _TIMER_H_
#define _TIMER_H_

namespace AwesomeRenderer
{
	
	struct TimingInfo
	{
		float elapsedSeconds;

		int totalFrames;
		int tickCount;
	};

	class Timer
	{
	public:

	private:

		TimingInfo lastInfo;

		float maxFrameTime;

		int framesThisSecond;

	public:
		Timer(float maxFrameTime = 1.0f);

		const TimingInfo& Tick();

	};

}

#endif