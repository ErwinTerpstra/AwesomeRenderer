#ifndef _CAMERA_CONTROLLER_H_
#define _CAMERA_CONTROLLER_H_

namespace AwesomeRenderer
{
	class Camera;

	class CameraController
	{

	public:
		Camera& camera;

	private:
		float yaw, pitch;

		float zoomSpeed, yawSpeed, pitchSpeed;

		float moveSpeed, strafeSpeed;

		float shiftMultiplier;

	public:
		CameraController(Camera& camera);

		void Update(const TimingInfo& timingInfo);

	};

}

#endif