#ifndef _MEMORY_BUFFER_H_
#define _MEMORY_BUFFER_H_

namespace AwesomeRenderer
{

	class MemoryBuffer : public Buffer
	{
		
	public:
		

	private:
		
	public:
		MemoryBuffer();
		~MemoryBuffer();
		
		virtual void Allocate(int preferredWidth, int preferredHeight, int stride);
		virtual void Destroy();

	};

}


#endif