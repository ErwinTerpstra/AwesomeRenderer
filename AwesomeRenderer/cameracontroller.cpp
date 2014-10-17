#include "awesomerenderer.h"

using namespace AwesomeRenderer;

CameraController::CameraController(Camera& camera) : 
	camera(camera), 
	yaw(0.2f), pitch(0.9f),
	yawSpeed(2.0f), pitchSpeed(2.0f),
	moveSpeed(2.0f), strafeSpeed(1.5f),
	shiftMultiplier(2.0f)
{

}


void CameraController::Update(const TimingInfo& timingInfo)
{
	InputManager& input = InputManager::Instance();
	float dt = timingInfo.elapsedSeconds;
	float multiplier = (input.GetKey(VK_SHIFT) ? shiftMultiplier : 1.0f) * dt;

	if (input.GetKey(VK_DOWN))
		pitch += pitchSpeed * dt;

	if (input.GetKey(VK_UP))
		pitch -= pitchSpeed * dt;

	if (input.GetKey(VK_LEFT))
		yaw += yawSpeed * dt;

	if (input.GetKey(VK_RIGHT))
		yaw -= yawSpeed * dt;

	if (input.GetKey('W'))
		camera.position += camera.Forward() * moveSpeed * multiplier;

	if (input.GetKey('S'))
		camera.position -= camera.Forward() * moveSpeed * multiplier;

	if (input.GetKey('D'))
		camera.position -= camera.Right() * strafeSpeed * multiplier;

	if (input.GetKey('A'))
		camera.position += camera.Right() * strafeSpeed * multiplier;

	camera.lookAt = camera.position + Vector3(std::cos(yaw) * std::sin(pitch),
											  std::cos(pitch),
											  std::sin(yaw) * std::sin(pitch));

}
