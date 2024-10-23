#pragma once

extern int32 MAX_CLIENT_COUNT;
extern int32 MAX_WORKER_COUNT;
extern int32 FPS;
extern class ConnectionManager* GConnectionManager;
extern class ConnectionPool* CP;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

private:
};

