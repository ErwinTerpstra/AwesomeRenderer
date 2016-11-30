#ifndef _MEMORY_BUFFER_ALLOCATOR_H_
#define _MEMORY_BUFFER_ALLOCATOR_H_

#include "awesomerenderer.h"
#include "bufferallocator.h"

namespace AwesomeRenderer
{

	class MemoryBufferAllocator : public BufferAllocator
	{
		
	public:
		

	private:
		uchar* data;

	public:
		MemoryBufferAllocator();
		~MemoryBufferAllocator();
		
		virtual uchar* Allocate(const Buffer& buffer);
		virtual void Destroy();

	};

}


#endif