
#include "awesomerenderer.h"

#include "sampler.h"

#include "texture.h"

using namespace AwesomeRenderer;

Sampler::Sampler(Texture* texture) : texture(texture), wrapMode(WM_DEFAULT), sampleMode(SM_DEFAULT)
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

Color Sampler::Sample(const Vector2& uv, float mipLevel) const
{
	if (!texture->HasMipmaps())
		return Sample(uv, 0U);

	Color sample;

	mipLevel = Util::Clamp(mipLevel, 0.0f, (float) texture->GetMipmapLevels());

	switch (sampleMode)
	{
		case SM_POINT:
		case SM_BILINEAR:
			Sample(uv, sample, (uint32_t) floor(mipLevel));
			break;

		case SM_TRILINEAR:
			uint32_t intMipLevel = (uint32_t) mipLevel;
			float fractMipLevel = mipLevel - intMipLevel;

			Color samples[2];
			Sample(uv, samples[0], intMipLevel);
			Sample(uv, samples[1], intMipLevel + (intMipLevel < texture->GetMipmapLevels() ? 1 : 0));
			
			sample = samples[0] * (1.0f - fractMipLevel) + samples[1] * fractMipLevel;
			break;
	}

	return sample;
}

Color Sampler::SampleMipMaps(const Vector2& uv, float distance, double surfaceAreaToTextureRatio, float screenResolution)
{
	float textureResolution = texture->GetResolution();

	float textureToScreenRatio = textureResolution / screenResolution;

	//float log2IdealDistance = 0.5f * log2(surfaceAreaToTextureRatio  / textureToScreenRatio);
	float log2IdealDistance = (float) log2(surfaceAreaToTextureRatio / textureToScreenRatio);

	float mipLevel = log2(distance) - log2IdealDistance;

	return Sample(uv, mipLevel);
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

			// TODO: If the wrap mode is WM_REPEAT, these coordinates should wrap instead of clamp
			uint32_t neighbourX = std::min(intX + 1, buffer->width - 1);
			uint32_t neighbourY = std::min(intY + 1, buffer->height - 1);

			float fractX = ts[0] - intX;
			float fractY = ts[1] - intY;

			Color samples[4];
			buffer->GetPixel(intX,			intY,		samples[0], Buffer::LINEAR);
			buffer->GetPixel(neighbourX,	intY,		samples[1], Buffer::LINEAR);
			buffer->GetPixel(intX,			neighbourY, samples[2], Buffer::LINEAR);
			buffer->GetPixel(neighbourX,	neighbourY, samples[3], Buffer::LINEAR);

			// Interpolate in X direction
			samples[0] = (samples[0] * (1.0f - fractX)) + (samples[1] * fractX);
			samples[1] = (samples[2] * (1.0f - fractX)) + (samples[3] * fractX);

			// Interpolate in Y direction
			sample = samples[0] * (1.0f - fractY) + samples[1] * fractY;

			break;
		}
	}
}