#include "awesomerenderer.h"

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
	filePtr = fopen(fileName.c_str(), "rb");
	if (filePtr == NULL)
	{
		printf("[Bitmap]: Failed to open file \"%s\"\n", fileName);
		return false;
	}

	// Read the bitmap file header
	fread(&fileHeader, sizeof(BmpFileHeader), 1, filePtr);

	int s = sizeof(BmpFileHeader);

	// Verify that this is a bmp file by check bitmap id
	if (fileHeader.type != 0x4D42)
	{
		printf("[Bitmap]: Invalid bitmap format\n");
		fclose(filePtr);

		return false;
	}

	// Read the bitmap info header
	fread(&infoHeader, sizeof(BmpInfoHeader), 1, filePtr);

	// Some editors don't write the sizeImage field
	if (infoHeader.sizeImage == 0)
		infoHeader.sizeImage = infoHeader.width * infoHeader.height * (infoHeader.bitCount / 8);

	// Allocate memory
	(*texture)->Allocate(infoHeader.width, infoHeader.height, infoHeader.bitCount / 8);

	// Read bitmap data
	fseek(filePtr, fileHeader.offBits, SEEK_SET);
	int bytesRead = fread((*texture)->data, sizeof(uchar), infoHeader.sizeImage, filePtr);
	
	printf("[Bitmap]: Loaded \"%s\" with %d bytes\n", fileName.c_str(), bytesRead);

	fclose(filePtr);
	
	return true;
}