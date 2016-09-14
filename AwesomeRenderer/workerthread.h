#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Scheduler;
	class WorkerGroup;

	class WorkerThread
	{
	private:
		
		HANDLE handle;

		DWORD id;

		WorkerGroup& group;

		bool running;

	public:
		WorkerThread(WorkerGroup& group);

		void Start();
		void Stop();

		bool IsRunning() const { return running; }

	private:

		DWORD Run();

		static DWORD WINAPI HandleWorker(LPVOID args);
	};
}

#endif