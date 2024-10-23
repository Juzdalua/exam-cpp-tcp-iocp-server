#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "ConnectionPool.h"

int32 MAX_CLIENT_COUNT = 1;
int32 MAX_WORKER_COUNT = thread::hardware_concurrency();

extern CoreGlobal GCoreGlobal;
ConnectionManager* GConnectionManager = nullptr;
ConnectionPool* CP = nullptr;

CoreGlobal::CoreGlobal()
{
	// Socket
	SocketUtils::Init();

	// DB
	CP = new ConnectionPool(5);
}

CoreGlobal::~CoreGlobal()
{
	// Socket
	SocketUtils::Clear();

	// DB
	delete CP;
}
