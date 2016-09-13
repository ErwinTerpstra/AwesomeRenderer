#include "workerthread.h"

using namespace AwesomeRenderer;

WorkerThread::WorkerThread() : running(false)
{

}

void WorkerThread::Start()
{
	running = true;
	
	// Create and start a thread for this worker
	handle = CreateThread(NULL, 0, WorkerThread::HandleWorker, this, 0, &id);
}

void WorkerThread::Stop()
{
	// Signal we are shutting down
	running = false;
}

DWORD WorkerThread::Run()
{
	while (IsRunning())
	{
		Sleep(1000);
	}

	return 0;
}

DWORD WINAPI WorkerThread::HandleWorker(LPVOID args)
{
	WorkerThread* thread = static_cast<WorkerThread*>(args);
	return thread->Run();
}