#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"

namespace procademy
{
	class CMonitorServer
	{
	public:

	private:
		SOCKET mListenSocket = INVALID_SOCKET;
		BYTE	mNumThreads = 0;
		HANDLE	mIOCP = INVALID_HANDLE_VALUE;
		HANDLE*  mThreads = nullptr;
		u_short				mPort = 0;
		WCHAR				mBindIP[32];
		S
	};
}