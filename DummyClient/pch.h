#pragma once

#define WIN32_LEAN_AND_MEAN   

#ifdef _DEBUG
#pragma comment(lib, "CoreLib\\Debug\\CoreLib.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "CoreLib\\Release\\CoreLib.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif 

#include "CorePch.h"

