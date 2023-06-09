
#include "awesomerenderer.h"
#include "texturefactory.h"
#include "memorybufferallocator.h"
#include "sampler.h"
#include "lodepng\lodepng.h"

using namespace AwesomeRenderer;

TextureFactory::TextureFactory()
{
	AddLoadFunction("bmp", &TextureFactory::LoadBMP);
	AddLoadFunction("png", &TextureFactory::LoadPNG);
}

bool TextureFactory::Instantiate(Texture** instance) const
{
	new (*instance) Texture(new MemoryBufferAllocator(), Buffer::GAMMA);

	return true;
}


bool TextureFactory::LoadRAW(const std::string& fileName, Buffer& buffer) const
{
	// TODO: Move binary reading of file to FileReader class
	FILE* filePtr;

	// Open filename in read binary mode 
	errno_t result = fopen_s(&filePtr, fileName.c_str(), "rb");

	if (result != 0)
	{
		printf("[TextureFactory]: Failed to open file \"%s\". Error code: %d\n", fileName.c_str(), result);
		return false;
	}

	uchar* destination = buffer.GetBase(0, 0);
	int readBytes = fread(destination, 1, buffer.size, filePtr);

	if (readBytes < buffer.size)
		printf("[TextureFactory]: Warning! File only contained %d bytes of the %u bytes the buffer expects\n", readBytes, buffer.size);

	fclose(filePtr);

	return true;
}

bool TextureFactory::LoadBMP(const std::string& fileName, Texture** texture) const
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

	case 32:
		encoding = Buffer::BGRA32;
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

	printf("[TextureFactory]: Loaded BMP \"%s\" with %d bytes\n", fileName.c_str(), bytesRead);

	fclose(filePtr);
		
	return true;
}

bool TextureFactory::LoadPNG(const std::string& fileName, Texture** texture) const
{
	(*texture)->Destroy();

	// Load the file from disk
	// TODO: implement binary file reader
	std::vector<uchar> png;
	uint32_t error = lodepng::load_file(png, fileName);

	// Decode the PNG
	lodepng::State state; //optionally customize this one

	std::vector<uchar> image;
	uint32_t width, height;

	if (!error)
		error = lodepng::decode(image, width, height, state, png);

	// Handle errors
	if (error)
	{
		printf("[TextureFactory]: Error while loading PNG (%d): %s\n", error, lodepng_error_text(error));
		return false;
	}

	// Allocate memory for the texture
	Buffer::Encoding encoding = Buffer::RGBA32;
	(*texture)->Allocate(width, height, encoding);

	// Copy the image to the texture, since PNG rows to top to bottom, instead of bottom to top, reverse the row order
	uchar* src = &image[0];
	uchar* dst = (*texture)->data;
	uint32_t stride = (*texture)->stride;

	for (uint32_t y = 0; y < height; ++y)
		memcpy(dst + (height - 1 - y) * stride, src + y * stride, stride);

	printf("[TextureFactory]: Loaded PNG \"%s\" with %d bytes\n", fileName.c_str(), png.size());

	return true;
}

void TextureFactory::PostProcessAsset(Texture* instance)
{
	instance->GenerateMipMaps();
}

Sampler* TextureFactory::GetTexture(const std::string& fileName)
{
	Texture* texture = NULL;
	
	if (!GetAsset(fileName, &texture))
		return NULL;
	
	return CreateSampler(texture);
}

Sampler* TextureFactory::CreateSampler(Texture* texture)
{
	Sampler* sampler = new Sampler();
	sampler->sampleMode = texture->HasMipmaps() ? Sampler::SM_TRILINEAR : Sampler::SM_BILINEAR;
	sampler->wrapMode = Sampler::WM_REPEAT;
	sampler->texture = texture;

	return sampler;
}

Texture* TextureFactory::MergeAlphaChannel(const Texture* albedo, const Texture* alpha)
{
	assert(albedo->width == alpha->width && albedo->height == alpha->height);

	Texture* target;
	
	if (!Allocate(&target))
		return NULL;

	target->AllocateAligned(albedo->width, albedo->height, albedo->alignment, Texture::RGBA32);

	for (uint32_t y = 0; y < albedo->height; ++y)
	{
		for (uint32_t x = 0; x < albedo->width; ++x)
		{
			Color albedoColor;
			albedo->GetPixel(x, y, albedoColor);

			Color alphaColor;
			alpha->GetPixel(x, y, alphaColor);

			albedoColor[3] = alphaColor[0];

			target->SetPixel(x, y, albedoColor);
		}
	}

	return target;
}

Texture* TextureFactory::ConvertHeightMapToNormalMap(const Texture* heightMap, float scale)
{
	Texture* target;

	if (!Allocate(&target))
		return NULL;

	target->colorSpace = Buffer::LINEAR;
	target->AllocateAligned(heightMap->width, heightMap->height, heightMap->alignment, Texture::RGB24);

	float s[9];

	for (int32_t y = 0; y < heightMap->height; ++y)
	{
		for (int32_t x = 0; x < heightMap->width; ++x)
		{
			// Read all samples for the 3x3 convolution kernel
			for (int32_t yOffset = -1; yOffset <= 1; ++yOffset)
			{
				for (int32_t xOffset = -1; xOffset <= 1; ++xOffset)
				{
					uint32_t sampleX = Util::Clamp(x + xOffset, 0, (int) heightMap->width - 1);
					uint32_t sampleY = Util::Clamp(y + yOffset, 0, (int) heightMap->height - 1);

					Color heightMapColor;
					heightMap->GetPixel(sampleX, sampleY, heightMapColor);

					// Use the red channel as height source
					s[(yOffset + 1) * 3 + (xOffset + 1)] = heightMapColor[0] - 0.5f;
				}
			}

			Vector3 normal;

			normal[0] = scale * -(s[2] - s[0] + 2 * (s[5] - s[3]) + s[8] - s[6]);
			normal[1] = scale * -(s[6] - s[0] + 2 * (s[7] - s[1]) + s[8] - s[2]);
			normal[2] = 1.0;
			
			normal = VectorUtil<3>::Normalize(normal);

			normal[0] = 0.5f + normal[0] / 2.0f;
			normal[1] = 0.5f + normal[1] / 2.0f;
			normal[2] = 0.5f + normal[2] / 2.0f;

			target->SetPixel(x, y, Color(normal));
		}

	}

	return target;
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