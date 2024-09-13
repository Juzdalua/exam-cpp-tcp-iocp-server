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

void GlobalSendQueue::DoAsync()
{
	JobRef job = nullptr;

	{
		lock_guard<mutex> lock(_lock);
		if (_jobs.empty())
			return;

		job = _jobs.front();
		_jobs.pop();
	}

	if (job == nullptr) {
		return;
	}

	job->Execute();
}