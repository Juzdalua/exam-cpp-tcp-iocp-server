#pragma once

class IJob
{
public :
	virtual void Execute() {}
};

using JobRef = shared_ptr<IJob>;

class JobQueue
{
public:
	void Push(JobRef job);
	JobRef Pop();

private:
	mutex _lock;
	queue<JobRef> _jobs;
};