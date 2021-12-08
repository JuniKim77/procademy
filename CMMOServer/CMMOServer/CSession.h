#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "TC_LFQueue.h"
#include "TC_LFStack.h"
#include "CSafeQueue.h"

namespace procademy
{
	class CNetPacket;

	class CSession
	{
		friend class CMMOServer;
	public:
		enum SESSION_STATUS
		{
			en_NONE_USE,
			en_AUTH_READY,
			en_AUTH_RUN,
			en_GAME_READY,
			en_GAME_RUN,
			en_AUTH_RELEASE,
			en_GAME_RELEASE
		};

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
		CSafeQueue<CNetPacket*>		recvCompleteQ;
		TC_LFQueue<CNetPacket*>		sendQ;
		int							numSendingPacket = 0;
		int							ioCount = 0;
		bool						isSending = false;
		SOCKET						socket = INVALID_SOCKET;
		u_short						port;
		ULONG						ip;
		u_int64						sessionID;
		bool						sessionEnd = false;
		bool						toGame = false;
		SESSION_STATUS				status;
	};
}