#include "awesomerenderer.h"
#include "memorybuffer.h"

using namespace AwesomeRenderer;

MemoryBuffer::MemoryBuffer() :  Buffer()
{

}

MemoryBuffer::~MemoryBuffer()
{

}

void MemoryBuffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, uint32_t stride)
{
	SetDimensions(preferredWidth, preferredHeight, stride);
	data = new uchar[this->size];
}

void MemoryBuffer::Destroy()
{
	if (data != NULL)
	{
		delete[] data;
		data = NULL;
	}
}