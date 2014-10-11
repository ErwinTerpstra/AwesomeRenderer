#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Counter::Counter() : m(), count(0), maxCount(0)
{

}

void Counter::Configure(uint32_t count, uint32_t maxCount)
{
	this->count = count;
	this->maxCount = maxCount;
}

void Counter::Reset()
{
	m.lock();
	count = maxCount;
	m.unlock();
}

void Counter::Decrement()
{
	m.lock();
	--count;

	signal.notify_all();
	m.unlock();
}

void Counter::WaitZero()
{
	// Gain access to the lock
	m.lock();

	while (true)
	{
		// Check if our continue condition has been met
		if (count == 0)
			break;

		// Release the lock and wait for an other thread to signal change in the count
		signal.wait(m);
	}

	// Release the lock since our wait condition is finished
	m.unlock();
}


Semaphore::Semaphore(uint32_t count, uint32_t maxCount) : count(count), maxCount(maxCount)
{

}

void Semaphore::Signal(uint32_t increment)
{
	m.lock();
	count = std::min(count + increment, maxCount);

	signal.notify_all();
	m.unlock();
}

void Semaphore::Wait()
{
	m.lock();

	if (count == 0)
		signal.wait(m);

	--count;

	m.unlock();
}