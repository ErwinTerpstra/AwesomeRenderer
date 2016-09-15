#include "stdafx.h"
#include "scheduler.h"

#include "workerthread.h"

using namespace AwesomeRenderer;

Scheduler::Scheduler(int sharedThreads) : running(false)
{
	SetupWorkers(sharedThreads, &mainGroup);
}

Scheduler::~Scheduler()
{
	for (auto it = workers.begin(); it != workers.end(); ++it)
		delete *it;

	for (auto it = jobGroups.begin(); it != jobGroups.end(); ++it)
		delete *it;

	workers.clear();
	jobGroups.clear();
}

void Scheduler::SetupWorkers(uint32_t threads, JobGroup* group)
{
	for (uint32_t threadIdx = 0; threadIdx < threads; ++threadIdx)
	{
		WorkerThread* worker = new WorkerThread(group);
		workers.push_back(worker);

		if (running)
			worker->Start();
	}
}

void Scheduler::Start()
{
	for (auto it = workers.begin(); it != workers.end(); ++it)
		(*it)->Start();

	running = true;
}

void Scheduler::Stop()
{
	for (auto it = workers.begin(); it != workers.end(); ++it)
		(*it)->Stop();

	mainGroup.jobSignal.Signal(workers.size());

	running = false;
}

void Scheduler::ScheduleJob(WorkerJob* job)
{
	mainGroup.EnqueueJob(job);
}

JobGroup* Scheduler::CreateJobGroup(uint32_t dedicatedThreads)
{
	JobGroup* group = new JobGroup();
	jobGroups.push_back(group);

	SetupWorkers(dedicatedThreads, group);

	return group;
}