#include "stdafx.h"
#include "awesomerenderer.h"
#include "texturefactory.h"
#include "sampler.h"

using namespace AwesomeRenderer;

TextureFactory::TextureFactory()
{
	AddLoadFunction("bmp", &TextureFactory::LoadBmp);
}

bool TextureFactory::Instantiate(Texture** instance) const
{
	new (*instance) Texture();

	return true;
}

bool TextureFactory::LoadBmp(const std::string& fileName, Texture** texture) const
{
	(*texture)->Destroy();
		
	BmpFileHeader fileHeader;
	BmpInfoHeader infoHeader;

	// TODO: Move binary reading of file to FileReader class
	FILE* filePtr;
	
	// Open filename in read binary mode 
	errno_t result = fopen_s(&filePtr, fileName.c_str(), "rb");

	if (result != 0)
	{
		printf("[TextureFactory]: Failed to open file \"%s\". Error code: %d\n", fileName.c_str(), result);
		return false;
	}

	// Read the bitmap file header
	fread(&fileHeader, sizeof(BmpFileHeader), 1, filePtr);

	int s = sizeof(BmpFileHeader);

	// Verify that this is a bmp file by check bitmap id
	if (fileHeader.type != 0x4D42)
	{
		printf("[TextureFactory]: Invalid bitmap format\n");
		fclose(filePtr);

		return false;
	}

	// Read the bitmap info header
	fread(&infoHeader, sizeof(BmpInfoHeader), 1, filePtr);

	// Some editors don't write the sizeImage field
	if (infoHeader.sizeImage == 0)
		infoHeader.sizeImage = infoHeader.width * infoHeader.height * (infoHeader.bitCount / 8);

	assert(infoHeader.width > 0 && infoHeader.height > 0 && infoHeader.bitCount > 0);
	assert(infoHeader.sizeImage > 0);

	// Convert bit count to an encoding type
	Buffer::Encoding encoding;

	switch (infoHeader.bitCount)
	{
	case 24:
		encoding = Buffer::BGR24;
		break;
	
	default:
		printf("[TextureFactory]: Unsupported bit depth: %d\n", infoHeader.bitCount);
		return false;

	}
	
	// Allocate memory
	(*texture)->Allocate(infoHeader.width, infoHeader.height, encoding);
	
	// Read bitmap data	
	fseek(filePtr, fileHeader.offBits, SEEK_SET);
	int bytesRead = fread((*texture)->data, sizeof(uchar), (*texture)->size, filePtr);
	
	assert(bytesRead == (*texture)->size);

	printf("[TextureFactory]: Loaded \"%s\" with %d bytes\n", fileName.c_str(), bytesRead);

	fclose(filePtr);
		
	return true;
}

Sampler* TextureFactory::GetTexture(const std::string& fileName)
{
	Texture* texture = NULL;
	
	if (!GetAsset(fileName, &texture))
		return NULL;

	Sampler* sampler = new Sampler();
	sampler->sampleMode = Sampler::SM_POINT;
	sampler->wrapMode = Sampler::WM_REPEAT;
	sampler->texture = texture;

	return sampler;
}

void TextureFactory::WriteBMP(const std::string& fileName, const Buffer& buffer) const
{
	assert(buffer.encoding == Buffer::BGR24 && "Unsupported bitmap encoding");

	// TODO: Move binary reading of file to FileReader class
	FILE* filePtr;

	// Open filename in write binary mode 
	errno_t result = fopen_s(&filePtr, fileName.c_str(), "wb");

	if (result != 0)
	{
		printf("[TextureFactory]: Failed to open file \"%s\". Error code: %d\n", fileName.c_str(), result);
		return;
	}

	const uint8_t bpp = 24;
	const uint32_t headerSize = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

	BmpFileHeader fileHeader;
	fileHeader.type = 0x4D42;
	fileHeader.size = buffer.size + headerSize;
	fileHeader.offBits = headerSize;
	fileHeader.reserved1 = 0;
	fileHeader.reserved2 = 0;

	BmpInfoHeader infoHeader;
	infoHeader.size = sizeof(BmpInfoHeader);
	infoHeader.width = buffer.width;
	infoHeader.height = buffer.height;
	infoHeader.planes = 1;
	infoHeader.compression = 0;			// RGB colors
	infoHeader.bitCount = bpp;
	infoHeader.sizeImage = buffer.size;
	infoHeader.xPelsPerMeter = 2835;	// ~72 DPI
	infoHeader.yPelsPerMeter = 2835;	// ~72 DPI
	infoHeader.clrUsed = 0;				// Default to 2 ^ bpp
	infoHeader.clrImportant = 0;		// All colors are important

	fwrite(&fileHeader, sizeof(BmpFileHeader), 1, filePtr);
	fwrite(&infoHeader, sizeof(BmpInfoHeader), 1, filePtr);

	fwrite(buffer.GetBase(0, 0), 1, buffer.size, filePtr);

	fclose(filePtr);
}