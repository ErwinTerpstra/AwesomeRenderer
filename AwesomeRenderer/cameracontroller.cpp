#include "awesomerenderer.h"

using namespace AwesomeRenderer;

CameraController::CameraController(Camera& camera) : 
	camera(camera), distance(1.0f),
	azimuth(0.0f), zenith((float) M_PI * 0.5f),
	azimuthSpeed(2.0f), zenithSpeed(2.0f)
{

}

void CameraController::Update(const TimingInfo& timingInfo)
{
	InputManager& input = InputManager::Instance();

	if (input.GetKey('W'))
		zenith -= zenithSpeed * timingInfo.elapsedSeconds;

	if (input.GetKey('S'))
		zenith += zenithSpeed * timingInfo.elapsedSeconds;

	if (input.GetKey('A'))
		azimuth -= azimuthSpeed * timingInfo.elapsedSeconds;

	if (input.GetKey('D'))
		azimuth += azimuthSpeed * timingInfo.elapsedSeconds;

	camera.position = camera.lookAt;
	camera.position += Vector3(std::cos(azimuth) * std::sin(zenith), 
							   std::cos(zenith),
							   std::sin(azimuth) * std::sin(zenith)) * distance;

}
