#ifndef _SAMPLER_H_
#define _SAMPLER_H_

namespace AwesomeRenderer
{

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

			SM_DEFAULT = SM_BILINEAR
		};

		WrapMode wrapMode;
		SampleMode sampleMode;
		Texture* texture;

	public:
		Sampler();

		Color Sample(const Vector2& uv) const;
		void Sample(const Vector2& uv, Color& sample) const;

	};

}

#endif