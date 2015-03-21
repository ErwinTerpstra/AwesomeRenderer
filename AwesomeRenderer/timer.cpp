#include "awesomerenderer.h"

#include "timer.h"

using namespace AwesomeRenderer;

Timer::Timer(float minFrameTime, float maxFrameTime) : minFrameTime(minFrameTime), maxFrameTime(maxFrameTime)
{

}

const TimingInfo& Timer::Tick()
{
	++lastInfo.totalFrames;

	int tickCount = GetTickCount();
	float frameTime = (tickCount - lastInfo.tickCount) / 1000.0f;

	lastInfo.elapsedSeconds = std::max(std::min(frameTime, maxFrameTime), minFrameTime);
	lastInfo.totalSeconds = tickCount / 1000.0f;
	lastInfo.tickCount = tickCount;

	return lastInfo;
}

float Timer::Poll()
{
	return (GetTickCount() - lastInfo.tickCount) / 1000.0f;
}