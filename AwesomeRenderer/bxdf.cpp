#include "stdafx.h"
#include "bxdf.h"

using namespace AwesomeRenderer;
using namespace AwesomeRenderer::RayTracing;

BxDF::BxDF()
{

}

void BxDF::SphericalToCartesian(float phi, float theta, Vector3& v) const
{
	// Convert to cartesian coordinates
	float sinTheta = sinf(theta);
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);

	v = Vector3(x, cosf(theta), z);

	assert(VectorUtil<3>::IsNormalized(v));
}

void BxDF::TransformSampleVector(const Vector3& n, const Vector3& in, Vector3& out) const
{
	assert(VectorUtil<3>::IsNormalized(in));
	assert(VectorUtil<3>::IsNormalized(n));

	// Create an orientation matrix that aligns with the surface normal
	Vector3 right, forward;
	//VectorUtil<3>::OrthoNormalize(n, in, right, forward);
	VectorUtil<3>::OrthoNormalize(n, right, forward);

	assert(VectorUtil<3>::IsNormalized(right));
	assert(VectorUtil<3>::IsNormalized(forward));

	assert(fabs(VectorUtil<3>::Dot(n, forward)) < 1e-5f);
	assert(fabs(VectorUtil<3>::Dot(n, right)) < 1e-5f);
	assert(fabs(VectorUtil<3>::Dot(right, forward)) < 1e-5f);

	Matrix33 transform(
		right[0], right[1], right[2],
		n[0], n[1], n[2],
		forward[0], forward[1], forward[2]
		);
	
	// Transform the sample to world space
	out = transform_vector(transform, in);

	assert(VectorUtil<3>::IsNormalized(out));
}

bool BxDF::CalculateHalfVector(const Vector3& wo, const Vector3& wi, Vector3& h)
{
	h = wo + wi;

	if (h.length_squared() < 1e-5f)
		return false;

	h = VectorUtil<3>::Normalize(h);
	return true;
}