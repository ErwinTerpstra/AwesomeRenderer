#ifndef _WORKER_GROUP_H_
#define _WORKER_GROUP_H_

#include "awesomerenderer.h"

#include "threading.h"

namespace AwesomeRenderer
{

	class WorkerJob;

	class WorkerGroup
	{
	public:
		Semaphore jobSignal;

	private:
		LockedVariable<std::deque<WorkerJob*>> jobQueue;

	public:
		WorkerGroup();

		void EnqueueJob(WorkerJob* job);
		WorkerJob* DequeueJob();
	};

}


#endif