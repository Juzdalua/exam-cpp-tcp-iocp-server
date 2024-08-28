#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"

CoreGlobal GCoreGlobal;
SendBufferManager* GSendBufferManager = nullptr;

CoreGlobal::CoreGlobal()
{
	cout << "Core Global()" << endl;
	GSendBufferManager = new SendBufferManager();
	//SocketUtils::Init();
}

CoreGlobal::~CoreGlobal()
{
	cout << "~Core Global()" << endl;
	delete GSendBufferManager;
	//SocketUtils::Clear();
}
