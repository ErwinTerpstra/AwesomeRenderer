#ifndef _CAMERA_CONTROLLER_H_
#define _CAMERA_CONTROLLER_H_

namespace AwesomeRenderer
{
	class Camera;

	class CameraController
	{

	public:
		Camera& camera;

		float distance;

	private:

		float azimuth, zenith;

		float azimuthSpeed, zenithSpeed;

	public:
		CameraController(Camera& camera);

		void Update(const TimingInfo& timingInfo);

	};

}

#endif