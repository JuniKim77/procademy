#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "CLF_LanServer.h"
#include <unordered_map>
#include "TC_LFObjectPool.h"
#include "MonitorServerDTO.h"

namespace procademy
{
	class CMonitorServer : public CLF_LanServer
	{
	public:
		CMonitorServer();
		virtual ~CMonitorServer();
		bool	BeginServer();
		void	WaitForThreadsFin();
		
	private:
		virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void OnClientJoin(SESSION_ID SessionID) override;
		virtual void OnClientLeave(SESSION_ID SessionID) override;
		virtual void OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

		void		Init();
		bool		DBProc();


		static unsigned int WINAPI	DBThread(LPVOID arg);

	private:
		HANDLE										mDBThread;
		std::unordered_map<u_int64, st_Player*>		mPlayerMap;
		SRWLOCK										mPlayerMapLock;
	};
}