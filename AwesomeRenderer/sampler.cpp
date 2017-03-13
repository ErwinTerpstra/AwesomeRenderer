#include "stdafx.h"
#include "awesomerenderer.h"

#include "sampler.h"

#include "texture.h"

using namespace AwesomeRenderer;

Sampler::Sampler() : texture(NULL), wrapMode(WM_DEFAULT), sampleMode(SM_DEFAULT)
{

}

Color Sampler::Sample(const Vector2& uv) const
{
	Color sample;
	Sample(uv, sample);

	return sample;
}

void Sampler::Sample(const Vector2& uv, Color& sample) const
{
	Vector2 ts(uv);

	// Normalize texture coordinates to 0 ... 1 range by using the wrap mode
	switch (wrapMode)
	{
	case WM_REPEAT:
		ts[0] -= (int)ts[0];
		ts[1] -= (int)ts[1];

		if (ts[0] < 0.0f)
			ts[0] += 1.0f;

		if (ts[1] < 0.0f)
			ts[1] += 1.0f;

		break;

	case WM_CLAMP:
		ts[0] = cml::clamp(ts[0], 0.0f, 1.0f);
		ts[1] = cml::clamp(ts[1], 0.0f, 1.0f);
		break;
	}

	// Convert UV coordinates to pixel coordinates
	ts[0] *= (texture->width - 1);
	ts[1] *= (texture->height - 1);

	switch (sampleMode)
	{
	case SM_POINT:
		texture->GetPixel((uint32_t)ts[0], (uint32_t)ts[1], sample, Buffer::LINEAR);
		break;

	case SM_BILINEAR:
	{
		uint32_t intX = (uint32_t)ts[0];
		uint32_t intY = (uint32_t)ts[1];
		float fractX = ts[0] - intX;
		float fractY = ts[1] - intY;

		Color samples[4];
		texture->GetPixel(intX    , intY    , samples[0], Buffer::LINEAR);
		texture->GetPixel(intX + 1, intY    , samples[1], Buffer::LINEAR);
		texture->GetPixel(intX    , intY + 1, samples[2], Buffer::LINEAR);
		texture->GetPixel(intX + 1, intY + 1, samples[3], Buffer::LINEAR);
		
		// Interpolate in X direction
		samples[0] = (samples[0] * (1.0f - fractX)) + (samples[1] * fractX);
		samples[1] = (samples[2] * (1.0f - fractX)) + (samples[3] * fractX);

		// Interpolate in Y direction
		sample = samples[0] * (1.0f - fractY) + samples[1] * fractY;

		// TODO: Move this to color class
		sample[0] = cml::clamp(sample[0], 0.0f, 1.0f);
		sample[1] = cml::clamp(sample[1], 0.0f, 1.0f);
		sample[2] = cml::clamp(sample[2], 0.0f, 1.0f);
		sample[3] = cml::clamp(sample[3], 0.0f, 1.0f);

		break;
	}

	}

}