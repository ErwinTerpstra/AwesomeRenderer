#include "stdafx.h"
#include "scheduler.h"

#include "workerthread.h"

using namespace AwesomeRenderer;

Scheduler::Scheduler(int workerThreads) : workerCount(workerThreads)
{
	SetupWorkers();
}

Scheduler::~Scheduler()
{
	for (workerIterator = workers.begin(); workerIterator != workers.end(); ++workerIterator)
		delete *workerIterator;

	workers.clear();
}

void Scheduler::SetupWorkers()
{
	while (workers.size() < workerCount)
	{
		WorkerThread* thread = new WorkerThread(mainGroup);
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

	mainGroup.jobSignal.Signal(workers.size());
}

void Scheduler::ScheduleJob(WorkerJob* job)
{
	mainGroup.EnqueueJob(job);
}