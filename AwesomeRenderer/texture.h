#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "awesomerenderer.h"
#include "memorybuffer.h"

namespace AwesomeRenderer
{

	class Texture : public MemoryBuffer, public Extendee<Texture>
	{
	public:

	public:
		Texture();

		Color Sample(const Vector2& uv) const;
		void Sample(const Vector2& uv, Color& sample) const;

	};

}

#endif