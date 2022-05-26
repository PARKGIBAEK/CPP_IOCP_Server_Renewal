// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#endif //PCH_H

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

// Server Core Libraries
#include "Types.h"
#include "CoreMacros.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "Container.h"
#include "NetAddress.h"

#include "RecvBuffer.h"
#include "SendBuffer.h"

#include "ObjectPool.h"
#include "MemoryPool.h"
#include "Allocators.h"
#include "MemoryManager.h"
#include "JobQueue.h"
//#include "GlobalQueue.h"


// STL
#include <iostream>
#include <filesystem>
#include <source_location>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <memory>

