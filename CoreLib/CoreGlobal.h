#pragma once

extern int32 MAX_CLIENT_COUNT;
extern int32 MAX_WORKER_COUNT;
extern class ConnectionPool* CP;
extern class GlobalSendQueue* GSendQueue;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

private:
};

