#ifndef _TEXTURE_FACTORY_H_
#define _TEXTURE_FACTORY_H_

#include "awesomerenderer.h"
#include "factory.h"
#include "texture.h"

namespace AwesomeRenderer
{
	class Texture;

	class TextureFactory : public Factory<TextureFactory, Texture>
	{

	public:
		

#pragma pack(push)
#pragma pack(2)
		struct BmpFileHeader 
		{
			unsigned short type;
			unsigned int size;
			unsigned short reserved1;
			unsigned short reserved2;
			unsigned int offBits;
		};
		
#pragma pack(2)
		struct BmpInfoHeader 
		{
			unsigned int size;
			long width;
			long height;
			unsigned short planes;
			unsigned short bitCount;
			unsigned int compression;
			unsigned int sizeImage;
			long xPelsPerMeter;
			long yPelsPerMeter;
			unsigned int clrUsed;
			unsigned int clrImportant;
		};
#pragma pack(pop)

	public:

		TextureFactory();
		
	protected:

		bool Instantiate(Texture** instance) const;
		bool LoadBmp(const std::string& fileName, Texture** texture) const;
	};

}

#endif