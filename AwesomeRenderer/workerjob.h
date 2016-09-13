#ifndef _WORKER_JOB_H_
#define _WORKER_JOB_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class WorkerJob
	{

	public:
		WorkerJob();

		void Execute();

	protected:
		virtual void Run() = 0;

	};
}

#endif