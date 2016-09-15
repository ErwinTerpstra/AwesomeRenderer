#ifndef _WORKER_JOB_H_
#define _WORKER_JOB_H_

#include "awesomerenderer.h"

#include "threading.h"

namespace AwesomeRenderer
{
	class WorkerJob
	{

	public:

	private:
		std::mutex m;
		std::condition_variable_any signal;

		// TODO: merge to State enum?
		bool running;
		bool completed;
		bool interrupted;

	public:
		WorkerJob();

		void Execute();

		virtual void Interrupt();
		
		virtual void Reset();

		void WaitForCompletion();

		bool IsRunning() const { return running; }
		bool IsCompleted() const { return completed; }
		bool IsInterrupted() const { return interrupted; }

	protected:
		virtual void Run() = 0;

	};
}

#endif