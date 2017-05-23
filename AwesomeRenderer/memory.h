#ifndef _MEMORY_H_
#define _MEMORY_H_

namespace AwesomeRenderer
{

	template<typename T>
	T* AllocateAligned(uint32_t alignment, uint32_t count = 1)
	{
		return (T*)_aligned_malloc(sizeof(T) * count, alignment);
	}

	template<typename T>
	void FreeAligned(T* instance)
	{
		instance->~T();
		_aligned_free(instance);
	}


}

#endif