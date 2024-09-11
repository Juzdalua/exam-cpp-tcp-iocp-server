#include "pch.h"
#include "Job.h"

void JobQueue::Push(JobRef job)
{
	lock_guard<mutex> lock(_lock);
	_jobs.push(job);
}

JobRef JobQueue::Pop()
{
	lock_guard<mutex> lock(_lock);
	if (_jobs.empty())
		return nullptr;
	
	JobRef job = _jobs.front();
	_jobs.pop();

	return job;
}
