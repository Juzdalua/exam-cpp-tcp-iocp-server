#pragma once

class IJob
{
public:
	virtual void Execute() {}
};

class JobQueue
{
public:
	JobQueue() = default;
	virtual ~JobQueue() {}

public:
	void Push(JobRef job);
	JobRef Pop();

protected:
	mutex _lock;
	queue<JobRef> _jobs;
};

class GlobalSendQueue : public JobQueue
{
public:
	GlobalSendQueue() = default;
	virtual ~GlobalSendQueue() {}

public:
	void DoAsync();
};