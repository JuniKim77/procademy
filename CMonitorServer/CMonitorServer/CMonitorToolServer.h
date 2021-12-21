#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "CNetServerNoLock.h"
#include <unordered_map>

namespace procademy
{
	struct st_MonitorClient
	{
		SESSION_ID						sessionNo = 0;
		bool							bLogin = false;
	};

	class CMonitorToolServer : public CLF_NetServer
	{
		friend class CMonitorServer;
	public:
		CMonitorToolServer();
		virtual ~CMonitorToolServer();
		bool	BeginServer();
		void	QuitMonitorServer();
		void	StopMonitorServer();
		void	StartMonitorServer();
		void	SendDataToAllClinet(CNetPacket* packet);

	private:
		virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void OnClientJoin(SESSION_ID SessionID) override;
		virtual void OnClientLeave(SESSION_ID SessionID) override;
		virtual void OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

		void				Init();
		bool				JoinProc(SESSION_ID sessionID);
		bool				RecvProc(SESSION_ID sessionID, CNetPacket* packet);
		bool				LeaveProc(SESSION_ID sessionID);
		bool				MonitorLoginProc(SESSION_ID sessionID, CNetPacket* packet);

		void				LoadInitFile(const WCHAR* fileName);
		st_MonitorClient*	FindMonitorTool(SESSION_ID sessionNo);
		void				InsertMonitorTool(SESSION_ID sessionNo, st_MonitorClient* monitor);
		void				DeleteMonitorTool(SESSION_ID sessionNo);	
		void				LockServer() { AcquireSRWLockExclusive(&mToolServerLock); }
		void				UnlockServer() { ReleaseSRWLockExclusive(&mToolServerLock); }

	private:
		SRWLOCK												mToolServerLock;
		std::unordered_map<u_int64, st_MonitorClient*>		mMonitorClients;
		char												mLoginSessionKey[33];
	};
}