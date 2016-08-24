#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "awesomerenderer.h"
#include "memorybuffer.h"

namespace AwesomeRenderer
{

	class Texture : public MemoryBuffer, public ExtensionProvider<Texture>
	{
	public:

		enum Extensions
		{
			TEXTURE_GL
		};

	public:
		Texture();
		
	};

}

#endif