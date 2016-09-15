#include "stdafx.h"
#include "workergroup.h"

using namespace AwesomeRenderer;


WorkerGroup::WorkerGroup() : jobSignal(0, UINT32_MAX), jobQueue()
{

}

void WorkerGroup::EnqueueJob(WorkerJob* job)
{
	jobQueue.Lock();
	
	jobQueue->push_back(job);

	jobQueue.Unlock();

	jobSignal.Signal();
}

WorkerJob* WorkerGroup::DequeueJob()
{
	jobQueue.Lock();

	WorkerJob* job = NULL;
	if (!jobQueue->empty())
	{
		job = jobQueue->front();
		jobQueue->pop_front();
	}

	jobQueue.Unlock();

	return job;
}