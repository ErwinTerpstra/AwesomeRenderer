#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Timer::Timer(float maxFrameTime) : maxFrameTime(maxFrameTime)
{

}

const TimingInfo& Timer::Tick()
{
	++lastInfo.totalFrames;

	int tickCount = GetTickCount();
	lastInfo.elapsedSeconds = std::min((tickCount - lastInfo.tickCount) / 1000.0f, maxFrameTime);
	lastInfo.tickCount = tickCount;

	return lastInfo;
}