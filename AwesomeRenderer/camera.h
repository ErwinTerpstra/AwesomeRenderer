#ifndef _CAMERA_H_
#define _CAMERA_H_

namespace AwesomeRenderer
{
	class Camera
	{
	public:
		Matrix44 viewMtx;
		Matrix44 projMtx;
		Matrix44 viewportMtx;

		Vector3 position, lookAt, up;

		float nearPlane, farPlane, fov, aspect;

	private:
		cml::Handedness handedness;

	public:
		Camera(cml::Handedness handedness);

		void UpdateViewMtx();

		// View matrix
		void SetLookAt(const Vector3& position, const Vector3& lookAt, const Vector3& up);

		// Projection matrix
		void SetPerspective(float fov, float aspect, float near, float far);
		void SetOrthographic(float width, float height, float near, float far);
		
		// Viewport matrix
		void SetViewport(float x, float y, float width, float height);

		void ViewportToRay(const Vector2& viewport, Ray& ray);
	};

}

#endif