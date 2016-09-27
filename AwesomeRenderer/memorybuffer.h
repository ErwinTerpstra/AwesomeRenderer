#ifndef _MEMORY_BUFFER_H_
#define _MEMORY_BUFFER_H_

#include "awesomerenderer.h"
#include "buffer.h"

namespace AwesomeRenderer
{

	class MemoryBuffer : public Buffer
	{
		
	public:
		

	private:
		
	public:
		MemoryBuffer();
		~MemoryBuffer();
		
		virtual void AllocateAligned(uint32_t preferredWidth, uint32_t preferredHeight, uint8_t alignment, Encoding encoding);
		virtual void Destroy();

	};

}


#endif