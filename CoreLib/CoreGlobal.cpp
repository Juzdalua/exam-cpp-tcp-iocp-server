#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"

CoreGlobal GCoreGlobal;

CoreGlobal::CoreGlobal()
{
	cout << "Core Global()" << endl;
	//SocketUtils::Init();
}

CoreGlobal::~CoreGlobal()
{
	cout << "~Core Global()" << endl;
	//SocketUtils::Clear();
}
