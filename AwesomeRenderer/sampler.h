#ifndef _SAMPLER_H_
#define _SAMPLER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Buffer;
	class Texture;

	class Sampler
	{
	public:

		enum WrapMode
		{
			WM_CLAMP,
			WM_REPEAT,

			WM_DEFAULT = WM_REPEAT
		};

		enum SampleMode
		{
			SM_POINT,
			SM_BILINEAR,
			SM_TRILINEAR,

			SM_DEFAULT = SM_BILINEAR
		};

		WrapMode wrapMode;
		SampleMode sampleMode;
		Texture* texture;

	public:
		Sampler(Texture* texture = NULL);

		Color Sample(const Vector2& uv, uint32_t mipLevel = 0) const;
		void Sample(const Vector2& uv, Color& sample, uint32_t mipLevel = 0) const;

		Color Sample(const Vector2& uv, float mipLevel) const;
		Color SampleMipMaps(const Vector2& uv, float distance, double texelToSurfaceAreaRatio, float screenResolution);


	private:
		static void SampleBuffer(const Buffer* buffer, const Vector2& uv, SampleMode sampleMode, Color& sample);

	};

}

#endif