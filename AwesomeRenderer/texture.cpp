#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Texture::Texture() : MemoryBuffer(), wrapMode(WM_DEFAULT)
{

}

Color Texture::Sample(const Vector2& uv) const
{
	Color sample;
	Sample(uv, sample);

	return sample;
}

void Texture::Sample(const Vector2& uv, Color& sample) const
{
	Vector2 ts(uv);

	// Normalize texture coordinates to 0 ... 1 range by using the wrap mode
	switch (wrapMode)
	{
	case Texture::WM_REPEAT:
		ts[0] -= (int)ts[0];
		ts[1] -= (int)ts[1];

		if (ts[0] < 0.0f)
			ts[0] += 1.0f;

		if (ts[1] < 0.0f)
			ts[1] += 1.0f;

		break;

	case Texture::WM_CLAMP:
		ts[0] = cml::clamp(ts[0], 0.0f, 1.0f);
		ts[1] = cml::clamp(ts[1], 0.0f, 1.0f);
		break;
	}

	// Calculate the pixel to which the coordinates correspond
	int x = (int)(ts[0] * (width - 1));
	int y = (int)(ts[1] * (height - 1));

	GetPixel(x, y, sample);
}