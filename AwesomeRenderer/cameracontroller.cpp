#include "stdafx.h"
#include "awesomerenderer.h"

#include "cameracontroller.h"
#include "inputmanager.h"
#include "timer.h"

using namespace AwesomeRenderer;

CameraController::CameraController(Camera& camera) : 
	camera(camera), 
	yaw(0.2f), pitch(0.0f),
	shiftMultiplier(1.0f)
{

}

void CameraController::CopyFromCamera()
{
	// Calculate yaw and pitch from current camera position and look-at
	Vector3 forward = camera.Forward();

	Vector3 horizontalForward = forward;
	horizontalForward[1] = 0;
	horizontalForward.normalize();

	yaw = acos(cml::dot(horizontalForward, Vector3(0.0f, 0.0f, -1.0f))) - (float)HALF_PI;
	pitch = acos(cml::dot(horizontalForward, forward)) + (float) HALF_PI;
}

void CameraController::Update(const TimingInfo& timingInfo)
{
	InputManager& input = InputManager::Instance();
	float dt = timingInfo.elapsedSeconds;

	if (input.GetKey(VK_SHIFT))
		shiftMultiplier += SHIFT_ACCELERATION * dt;
	else
		shiftMultiplier = MIN_SHIFT_MULTIPLIER;

	if (input.GetKey(VK_DOWN))
		pitch += PITCH_SPEED * dt;

	if (input.GetKey(VK_UP))
		pitch -= PITCH_SPEED * dt;

	if (input.GetKey(VK_LEFT))
		yaw += YAW_SPEED * dt;

	if (input.GetKey(VK_RIGHT))
		yaw -= YAW_SPEED * dt;

	float multiplier = shiftMultiplier * dt;

	if (input.GetKey('W'))
		camera.position += camera.Forward() * MOVE_SPEED * multiplier;

	if (input.GetKey('S'))
		camera.position -= camera.Forward() * MOVE_SPEED * multiplier;

	if (input.GetKey('D'))
		camera.position -= camera.Right() * STRAFE_SPEED * multiplier;

	if (input.GetKey('A'))
		camera.position += camera.Right() * STRAFE_SPEED * multiplier;

	if (input.GetKey('E'))
		camera.position += camera.Up() * STRAFE_SPEED * multiplier;

	if (input.GetKey('Q'))
		camera.position -= camera.Up() * STRAFE_SPEED * multiplier;

	camera.lookAt = camera.position + Vector3(std::cos(yaw) * std::sin(pitch),
											  std::cos(pitch),
											  std::sin(yaw) * std::sin(pitch));

}
