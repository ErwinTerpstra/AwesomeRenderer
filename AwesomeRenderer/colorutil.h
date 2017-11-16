#ifndef _COLOR_UTIL_H_
#define _COLOR_UTIL_H_

namespace AwesomeRenderer
{

	class ColorUtil
	{

	public:
		static void Blend(const Color& src, const Color& dst, Color& out)
		{
			for (uint8_t channel = 0; channel < 3; ++channel)
				out[channel] = (src[channel] * src[3]) + (dst[channel] * (1.0f - src[3]));

		}
	};
}

#endif