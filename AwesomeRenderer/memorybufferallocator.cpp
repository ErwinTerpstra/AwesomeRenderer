
#include "awesomerenderer.h"
#include "memorybufferallocator.h"

using namespace AwesomeRenderer;

MemoryBufferAllocator::MemoryBufferAllocator() :  BufferAllocator(), data(NULL)
{

}

MemoryBufferAllocator::~MemoryBufferAllocator()
{

}

uchar* MemoryBufferAllocator::Allocate(const Buffer& buffer)
{	
	data = new uchar[buffer.size];
	
	return data;
}

void MemoryBufferAllocator::Destroy()
{
	if (data != NULL)
	{
		delete[] data;
		data = NULL;
	}
}