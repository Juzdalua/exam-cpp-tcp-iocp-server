#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "ConnectionPool.h"
#include "Job.h"

extern CoreGlobal GCoreGlobal;
ConnectionPool* CP = nullptr;
GlobalSendQueue* GSendQueue = nullptr;

CoreGlobal::CoreGlobal()
{
	// Socket
	SocketUtils::Init();

	// DB
	CP = new ConnectionPool(5);

	// SendQueue
	GSendQueue = new GlobalSendQueue();
}

CoreGlobal::~CoreGlobal()
{
	// Socket
	SocketUtils::Clear();

	// DB
	delete CP;

	// SendQueue
	delete GSendQueue;
}
