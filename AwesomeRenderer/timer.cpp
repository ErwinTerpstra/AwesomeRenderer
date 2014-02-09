#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Timer::Timer(float minFrameTime, float maxFrameTime) : minFrameTime(minFrameTime), maxFrameTime(maxFrameTime)
{

}

const TimingInfo& Timer::Tick()
{
	++lastInfo.totalFrames;

	int tickCount = GetTickCount();
	lastInfo.elapsedSeconds = std::max(std::min((tickCount - lastInfo.tickCount) / 1000.0f, maxFrameTime), minFrameTime);
	lastInfo.tickCount = tickCount;

	return lastInfo;
}