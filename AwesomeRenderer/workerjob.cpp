#include "stdafx.h"
#include "workerjob.h"

using namespace AwesomeRenderer;

WorkerJob::WorkerJob() : completed(false)
{

}

void WorkerJob::Execute()
{
	assert(!completed && "Completed jobs have to be reset first!");

	running = true;

	Run();

	running = false;
	completed = true;

	signal.notify_all();
}

void WorkerJob::Interrupt()
{
	interrupted = true;
}

void WorkerJob::Reset()
{
	Interrupt();
	WaitForCompletion();

	completed = false;
	interrupted = false;
}

void WorkerJob::WaitForCompletion()
{
	if (!running)
		return;

	m.lock();

	while (!completed)
		signal.wait(m);

	m.unlock();
}