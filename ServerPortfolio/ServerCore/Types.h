#pragma once


using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

#define size16(val)		static_cast<int16>(sizeof(val))
#define size32(val)		static_cast<int32>(sizeof(val))
#define size64(val)		static_cast<int64>(sizeof(val))
#define len16(val)	static_cast<int16>(sizeof(val)/sizeof(val[0]))
#define len32(val)	static_cast<int32>(sizeof(val)/sizeof(val[0]))
#define len64(val)	static_cast<int64>(sizeof(val)/sizeof(val[0]))


// 아래 매크로 사용 시 선언한 Type에 대한 Rename 기능이 제대로 동작하지 않으므로 주의!

/*
#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>;
// 사용 예시 : IocpServiceRef
USING_SHARED_PTR(IocpService);
USING_SHARED_PTR(ISession);
USING_SHARED_PTR(Session);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(ServerService);
USING_SHARED_PTR(ClientService);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(SendBufferChunk);
USING_SHARED_PTR(Job);
USING_SHARED_PTR(JobQueue);
*/