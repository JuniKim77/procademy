#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "TC_LFQueue.h"
#include "TC_LFStack.h"

namespace procademy
{
	class CNetPacket;

	struct SessionIoCount
	{
		union
		{
			LONG ioCount = 0;
			struct
			{
				SHORT count;
				SHORT isReleased;
			} releaseCount;
		};
	};

	class CSession
	{
	public:
		CSession();
		virtual ~CSession();

		virtual void OnAuth_ClientJoin() = 0;
		virtual void OnAuth_ClientLeave() = 0;
		virtual void OnAuth_ClientRelease() = 0;
		virtual void OnAuth_Update() = 0;
		virtual void OnAuth_Packet(CNetPacket* packet) = 0;
		virtual void OnGame_ClientJoin() = 0;
		virtual void OnGame_ClientLeave() = 0;
		virtual void OnGame_ClientRelease() = 0;
		virtual void OnGame_Update() = 0;
		virtual void OnGame_Packet(CNetPacket* packet) = 0;
		virtual void OnError(int errorcode, const WCHAR* log) = 0;

	private:
		WSAOVERLAPPED				recvOverlapped;
		WSAOVERLAPPED				sendOverlapped;
		RingBuffer					recvQ;
		TC_LFQueue<CNetPacket*>		sendQ;
		int							numSendingPacket = 0;
		alignas(64) SessionIoCount	ioBlock;
		alignas(64) bool			isSending = false;
		SOCKET						socket = INVALID_SOCKET;
		u_short						port;
		ULONG						ip;
	};
}