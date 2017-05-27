#include "stdafx.h"
#include "awesomerenderer.h"

#include "sampler.h"

#include "texture.h"

using namespace AwesomeRenderer;

const float Sampler::DEFAULT_MIP_DISTANCE_SCALE = 2000.0f;

Sampler::Sampler(Texture* texture) : texture(texture), wrapMode(WM_DEFAULT), sampleMode(SM_DEFAULT), mipDistanceScale(DEFAULT_MIP_DISTANCE_SCALE)
{

}

Color Sampler::Sample(const Vector2& uv, uint32_t mipLevel) const
{
	Color sample;
	Sample(uv, sample, mipLevel);

	return sample;
}

void Sampler::Sample(const Vector2& uv, Color& sample, uint32_t mipLevel) const
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

	SampleBuffer(texture->GetMipLevel(mipLevel), ts, sampleMode, sample);
}

Color Sampler::Sample(const Vector2& uv, float distance) const
{
	Color sample;

	float mipLevel = CalculateMipLevel(distance);

	switch (sampleMode)
	{
		case SM_POINT:
		case SM_BILINEAR:
			Sample(uv, sample, round(mipLevel * texture->GetMipmapLevels()));
			break;

		case SM_TRILINEAR:
			mipLevel *= (texture->GetMipmapLevels() + 1);

			uint32_t roundedMipLevel = mipLevel;
			float fraction = mipLevel - roundedMipLevel;

			Color samples[2];
			Sample(uv, samples[0], roundedMipLevel);
			Sample(uv, samples[1], roundedMipLevel + (roundedMipLevel < texture->GetMipmapLevels() ? 1 : 0));
			
			sample = samples[0] * (1.0f - fraction) + samples[1] * fraction;
			break;
	}

	return sample;
}

float Sampler::CalculateMipLevel(float distance) const
{
	float mipLevel;
	if (texture->HasMipmaps() && distance > 0)
	{
		float distanceFactor = 1.0f / (1.0f + (distance / mipDistanceScale));
		mipLevel = (1.0f - distanceFactor);
	}
	else
		mipLevel = 0;

	return mipLevel;
}

void Sampler::SampleBuffer(const Buffer* buffer, const Vector2& uv, SampleMode sampleMode, Color& sample)
{
	Vector2 ts(uv);

	// Convert UV coordinates to pixel coordinates
	ts[0] *= (buffer->width - 1);
	ts[1] *= (buffer->height - 1);

	switch (sampleMode)
	{
		case SM_POINT:
		{
			buffer->GetPixel((uint32_t)ts[0], (uint32_t)ts[1], sample, Buffer::LINEAR);
			break;
		}

		case SM_TRILINEAR:
		case SM_BILINEAR:
		{
			uint32_t intX = (uint32_t)ts[0];
			uint32_t intY = (uint32_t)ts[1];
			float fractX = ts[0] - intX;
			float fractY = ts[1] - intY;

			Color samples[4];
			buffer->GetPixel(intX, intY, samples[0], Buffer::LINEAR);
			buffer->GetPixel(intX + 1, intY, samples[1], Buffer::LINEAR);
			buffer->GetPixel(intX, intY + 1, samples[2], Buffer::LINEAR);
			buffer->GetPixel(intX + 1, intY + 1, samples[3], Buffer::LINEAR);

			// Interpolate in X direction
			samples[0] = (samples[0] * (1.0f - fractX)) + (samples[1] * fractX);
			samples[1] = (samples[2] * (1.0f - fractX)) + (samples[3] * fractX);

			// Interpolate in Y direction
			sample = samples[0] * (1.0f - fractY) + samples[1] * fractY;

			break;
		}
	}
}