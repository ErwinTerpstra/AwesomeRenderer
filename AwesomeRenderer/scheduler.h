#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "awesomerenderer.h"

#include "threading.h"

#include "workergroup.h"

namespace AwesomeRenderer
{
	class WorkerThread;
	class WorkerJob;

	class Scheduler
	{
	public:

	private:
		int workerCount;

		std::vector<WorkerThread*> workers;
		std::vector<WorkerThread*>::iterator workerIterator;

		WorkerGroup mainGroup;

	public:
		Scheduler(int workerCount);
		~Scheduler();

		void Start();
		void Stop();

		void ScheduleJob(WorkerJob* job);


	private:
		void SetupWorkers();

	};

}

#endif