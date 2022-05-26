#pragma once

#define WIN32_LEAN_AND_MEAN

#if _DEBUG
#if _WIN64
#pragma comment(lib,"CoreLibs\\x64\\Debug\\ServerCore.lib")
#elif _WIN32
#pragma comment(lib,"CoreLibs\\x32\\Release\\ServerCore.lib")
#endif
#pragma comment(lib,"Protobuf\\Debug\\libprotobufd.lib")
#elif
#if _WIN64
#pragma comment(lib,"CoreLibs\\x64\\Release\\ServerCore.lib")
#elif _WIN32
#pragma comment(lib,"CoreLibs\\x32\\Release\\ServerCore.lib")
#endif
#pragma comment(lib,"Protobuf\\Release\\libprotobuf.lib")
#endif

#include "Core.h"