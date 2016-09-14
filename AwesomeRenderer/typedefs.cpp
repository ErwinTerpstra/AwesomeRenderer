#include "stdafx.h"
#include "typedefs.h"

using namespace AwesomeRenderer;

Vector3 AwesomeRenderer::operator-(const float& lhs, const Vector3& rhs)
{
	return Vector3(lhs - rhs[0], lhs - rhs[1], lhs - rhs[2]);
}

Vector3 AwesomeRenderer::operator*(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2]);
}