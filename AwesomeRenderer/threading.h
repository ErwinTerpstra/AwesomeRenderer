#ifndef _THREADING_H_
#define _THREADING_H_

namespace AwesomeRenderer
{
	// TODO: Find a better name for this class (Lockable? SingleAccess?)
	template<typename T>
	class LockedVariable
	{
	private:
		T value;

		std::mutex mtx;

	public:
		LockedVariable() : value()
		{

		}

		LockedVariable(const T& initialValue) : value(initialValue)
		{

		}

		void Lock() { mtx.lock(); }
		void Unlock() { mtx.unlock(); }

		T& operator*() { return value; }
		const T& operator*() const { return value; }

		T* operator->() { return &value; }
		const T* operator->() const { return &value; }
	};

	// TODO: Find a better name for this class (standardized names?)
	class Counter
	{
	private:
		std::mutex m;
		std::condition_variable_any condition;

		uint32_t count, maxCount;
	public:
		Counter();

		void Configure(uint32_t count, uint32_t maxCount);

		void Reset();
		void Decrement();
		void WaitZero();
	};

	class Semaphore
	{
	private:
		std::mutex m;
		std::condition_variable_any condition;

		uint32_t count;
		const uint32_t maxCount;
	public:
		Semaphore(uint32_t count, uint32_t maxCount);
		void Signal(uint32_t increment = 1);
		void Wait();
	};
}

#endif