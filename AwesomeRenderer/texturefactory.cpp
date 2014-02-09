#include "awesomerenderer.h"

using namespace AwesomeRenderer;

TextureFactory::TextureFactory()
{

}

bool TextureFactory::Load(const char* fileName, Texture& texture) const
{
	const char* extension = strrchr(fileName, '.');

	// Select correct loading function based on file extension
	if (extension)
	{
		if (strcmp(extension, ".bmp") == 0)
			return LoadBmp(fileName, texture);

	}

	printf("[TextureFactory]: Unsupported file format for file \"%s\"\n", fileName);
	return false;
}

bool TextureFactory::LoadBmp(const char* fileName, Texture& texture) const
{
	texture.Destroy();
		
	BmpFileHeader fileHeader;
	BmpInfoHeader infoHeader;

	// TODO: Move binary reading of file to FileReader class
	FILE* filePtr;

	// Open filename in read binary mode
	filePtr = fopen(fileName, "rb");
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

	// Allocate memory
	texture.Allocate(infoHeader.width, infoHeader.height, infoHeader.bitCount / 8);

	// Read bitmap data
	fseek(filePtr, fileHeader.offBits, SEEK_SET);
	int bytesRead = fread(texture.data, sizeof(uchar), infoHeader.sizeImage, filePtr);
	
	printf("[Bitmap]: Loaded \"%s\" with %d bytes\n", fileName, bytesRead);

	fclose(filePtr);
	
	return true;
}