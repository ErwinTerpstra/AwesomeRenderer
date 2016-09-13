#include "scheduler.h"

#include "workerthread.h"

using namespace AwesomeRenderer;

Scheduler::Scheduler(int workerThreads) : workerCount(workerThreads)
{
	SetupWorkers();
}

Scheduler::~Scheduler()
{
	std::vector<WorkerThread*>::iterator it;

	for (workerIterator = workers.begin(); workerIterator != workers.end(); ++workerIterator)
		delete *it;

	workers.clear();
}

void Scheduler::SetupWorkers()
{
	while (workers.size() < workerCount)
	{
		WorkerThread* thread = new WorkerThread();
		workers.push_back(thread);
	}
}

void Scheduler::Start()
{
	for (workerIterator = workers.begin(); workerIterator != workers.end(); ++workerIterator)
		(*workerIterator)->Start();
}

void Scheduler::Stop()
{
	for (workerIterator = workers.begin(); workerIterator != workers.end(); ++workerIterator)
		(*workerIterator)->Stop();
}