#ifndef _CAMERA_CONTROLLER_H_
#define _CAMERA_CONTROLLER_H_

#include "awesomerenderer.h"
#include "camera.h"

namespace AwesomeRenderer
{
	class Camera;
	struct TimingInfo;

	class CameraController
	{

	public:
		Camera& camera;

	private:
		const float YAW_SPEED = 1.0f, PITCH_SPEED = 0.8f, MOVE_SPEED = 3.0f, STRAFE_SPEED = 2.0f;
		const float MIN_SHIFT_MULTIPLIER = 1.8f, SHIFT_ACCELERATION = 4.0f;
		
		float yaw, pitch;

		float shiftMultiplier;

	public:
		CameraController(Camera& camera);

		void CopyFromCamera();
		void Update(const TimingInfo& timingInfo);

	};

}

#endif