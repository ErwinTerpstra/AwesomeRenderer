#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class WorkerThread;

	class Scheduler
	{
	public:

	private:
		int workerCount;

		std::vector<WorkerThread*> workers;
		std::vector<WorkerThread*>::iterator workerIterator;

	public:
		Scheduler(int workerCount);
		~Scheduler();

		void Start();
		void Stop();

	private:
		void SetupWorkers();

	};

}

#endif