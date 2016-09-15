#ifndef _WORKER_GROUP_H_
#define _WORKER_GROUP_H_

#include "awesomerenderer.h"

#include "threading.h"

namespace AwesomeRenderer
{

	class WorkerJob;

	class JobGroup
	{
	public:
		Semaphore jobSignal;

	private:
		LockedVariable<std::deque<WorkerJob*>> jobQueue;

	public:
		JobGroup();

		void EnqueueJob(WorkerJob* job);
		WorkerJob* DequeueJob();

		void ClearQueue();
	};

}


#endif