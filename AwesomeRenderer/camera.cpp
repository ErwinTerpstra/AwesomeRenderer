#include "stdafx.h"
#include "awesomerenderer.h"
#include "camera.h"
#include "ray.h"

using namespace AwesomeRenderer;

Camera::Camera(cml::Handedness handedness) :
	position(0.0f, 0.0f, 0.0f), lookAt(0.0f, 0.0f, 1.0f), up(0.0f, 1.0f, 0.0f),
	apertureSize(0.0f), focalDistance(1.0f),
	handedness(handedness)
{
	UpdateViewMtx();

	projMtx.identity();
	viewportMtx.identity();
}

Vector3 Camera::Forward() const
{
	Vector3 forward = (lookAt - position);
	forward.normalize();

	return forward;
}

Vector3 Camera::Right() const
{
	return cml::cross(Forward(), Up());
}

Vector3 Camera::Up() const
{
	return up;
}

void Camera::UpdateViewMtx()
{
	cml::matrix_look_at(viewMtx, position, lookAt, up, handedness);
}
		
void Camera::SetLookAt(const Vector3& position, const Vector3& lookAt, const Vector3& up)
{
	this->position = position;
	this->lookAt = lookAt;
	this->up = up;

	UpdateViewMtx();
}

void Camera::SetPerspective(float fov, float aspect, float near, float far)
{
	this->fov = fov;
	this->aspect = aspect;
	this->nearPlane = near;
	this->farPlane = far;

	cml::matrix_perspective_xfov(projMtx, fov, aspect, near, far, handedness, cml::z_clip_zero);
}

void Camera::SetOrthographic(float width, float height, float near, float far)
{
	this->fov = 1.0f;
	this->aspect = width / height;
	this->nearPlane = near;
	this->farPlane = far;

	cml::matrix_orthographic(projMtx, width, height, near, far, handedness, cml::z_clip_zero);
}
		
void Camera::SetViewport(float x, float y, float width, float height)
{
	cml::matrix_viewport(viewportMtx, x, width, y, height, cml::z_clip_zero, 0.0f, 1.0f);
}

void Camera::ViewportToRay(const Vector2& viewport, Ray& ray)
{
	Vector3 origin, direction;
	cml::make_pick_ray(viewport[0], viewport[1], viewMtx, projMtx, viewportMtx, origin, direction);

	ray = Ray(origin, direction);
}