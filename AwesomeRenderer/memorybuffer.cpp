#include "stdafx.h"
#include "awesomerenderer.h"
#include "memorybuffer.h"

using namespace AwesomeRenderer;

MemoryBuffer::MemoryBuffer() :  Buffer()
{

}

MemoryBuffer::~MemoryBuffer()
{

}

void MemoryBuffer::AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding)
{	
	Buffer::AllocateAligned(preferredWidth, preferredHeight, alignment, encoding);

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