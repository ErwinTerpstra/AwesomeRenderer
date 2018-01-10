
#include "awesomerenderer.h"

#include "threading.h"

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

	condition.notify_all();
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
		condition.wait(m);
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

	// Increase the count
	count = std::min(count + increment, maxCount);

	// Notify all waiting threads
	condition.notify_all();

	m.unlock();
}

void Semaphore::Wait()
{
	m.lock();

	// Wait until the count increases
	while (count == 0)
		condition.wait(m);

	--count;

	m.unlock();
}
