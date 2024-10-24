#pragma once

#include "Types.h"
#include "CoreMacro.h"
#include "CoreGlobal.h"
#include "CoreTLS.h"

#include <string>
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include <windows.h>
#include <iostream>
using namespace std;

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Session.h"
#include "SendBuffer.h"
#include "DebugLog.h"
