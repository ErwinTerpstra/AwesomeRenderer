#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "awesomerenderer.h"

#include "threading.h"

#include "jobgroup.h"

namespace AwesomeRenderer
{
	class WorkerThread;

	class Scheduler
	{
	public:

	private:
		std::vector<WorkerThread*> workers;

		JobGroup mainGroup;
		std::vector<JobGroup*> jobGroups;

		bool running;

	public:
		Scheduler(int sharedThreads);
		~Scheduler();

		void Start();
		void Stop();

		void ScheduleJob(WorkerJob* job);

		JobGroup* CreateJobGroup(uint32_t dedicatedThreads = 0);

	private:
		void SetupWorkers(uint32_t threadCount, JobGroup* group = NULL);

	};

}

#endif