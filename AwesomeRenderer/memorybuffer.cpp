#include "awesomerenderer.h"

using namespace AwesomeRenderer;

MemoryBuffer::MemoryBuffer() :  Buffer()
{

}

MemoryBuffer::~MemoryBuffer()
{

}

void MemoryBuffer::Allocate(int preferredWidth, int preferredHeight, int stride)
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