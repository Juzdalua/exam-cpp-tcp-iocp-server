#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "ConnectionPool.h"

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
