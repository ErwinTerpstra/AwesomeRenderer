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

void MemoryBuffer::Allocate(uint32_t preferredWidth, uint32_t preferredHeight, Encoding encoding)
{	
	Buffer::Allocate(preferredWidth, preferredHeight, encoding);

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