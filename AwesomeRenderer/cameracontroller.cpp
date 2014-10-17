#include "awesomerenderer.h"

using namespace AwesomeRenderer;

CameraController::CameraController(Camera& camera) : 
	camera(camera), 
	yaw(0.2f), pitch(0.0f),
	yawSpeed(2.0f), pitchSpeed(2.0f),
	moveSpeed(2.0f), strafeSpeed(1.5f),
	shiftMultiplier(2.0f)
{

}

void CameraController::CopyFromCamera()
{
	// Calculate yaw and pitch from current camera position and look-at
	Vector3 forward = camera.Forward();

	Vector3 horizontalForward = forward;
	horizontalForward[1] = 0;
	horizontalForward.normalize();

	yaw = acos(cml::dot(horizontalForward, Vector3(0.0f, 0.0f, -1.0f))) - HALF_PI;
	pitch = acos(cml::dot(horizontalForward, forward)) + HALF_PI;
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
