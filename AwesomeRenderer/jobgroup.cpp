#include "stdafx.h"
#include "jobgroup.h"

using namespace AwesomeRenderer;


JobGroup::JobGroup() : jobSignal(0, UINT32_MAX), jobQueue()
{

}

void JobGroup::EnqueueJob(WorkerJob* job)
{
	jobQueue.Lock();
	
	jobQueue->push_back(job);

	jobQueue.Unlock();

	jobSignal.Signal();
}

WorkerJob* JobGroup::DequeueJob()
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


void JobGroup::ClearQueue()
{
	jobQueue.Lock();
	
	jobQueue->clear();

	jobQueue.Unlock();
}