
#include "awesomerenderer.h"

#include "transformation.h"

using namespace AwesomeRenderer;

Transformation::Transformation() : 
	position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f, 1.0f, 1.0f),
		parent(NULL), dirty(true)
{

}

void Transformation::SetPosition(const Vector3& position)
{
	this->position = position;
	this->dirty = true;
}


void Transformation::SetRotation(const Quaternion& rotation)
{
	this->rotation = rotation;
	this->dirty = true;
}

void Transformation::SetScale(const Vector3& scale)
{
	this->scale = scale;
	this->dirty = true;
}

const Matrix44& Transformation::WorldMtx() const
{
	return worldMtx;
}

const Matrix44& Transformation::LocalMtx() const
{
	return localMtx;
}

void Transformation::CalculateMtx()
{
	if (!dirty)
		return;

	Matrix44 t, r, s;
	cml::matrix_translation(t, position);
	cml::matrix_rotation_quaternion(r, rotation);
	cml::matrix_scale(s, scale);

	Matrix44 parentMtx;

	if (parent == NULL)
		parentMtx.identity();
	else
		parentMtx = parent->WorldMtx();

	localMtx = s * r * t;
	worldMtx = localMtx * parentMtx;

	dirty = false;
}