#include "awesomerenderer.h"

using namespace AwesomeRenderer;

CameraController::CameraController(Camera& camera) : 
	camera(camera), 
	distance(20.0f), zoomSpeed(6.0f),
	yaw(0.2f), pitch(0.9f),
	yawSpeed(2.0f), pitchSpeed(2.0f)
{

}

void CameraController::Update(const TimingInfo& timingInfo)
{
	InputManager& input = InputManager::Instance();
	float dt = timingInfo.elapsedSeconds;

	if (input.GetKey('W'))
		pitch -= pitchSpeed * dt;

	if (input.GetKey('S'))
		pitch += pitchSpeed * dt;

	if (input.GetKey('A'))
		yaw -= yawSpeed * dt;

	if (input.GetKey('D'))
		yaw += yawSpeed * dt;

	if (input.GetKey('Q'))
		distance += zoomSpeed * dt;

	if (input.GetKey('E'))
		distance -= zoomSpeed * dt;

	camera.position = camera.lookAt;
	camera.position += Vector3(std::cos(yaw) * std::sin(pitch),
							   std::cos(pitch),
							   std::sin(yaw) * std::sin(pitch)) * distance;

}
