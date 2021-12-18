#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "CLF_LanServer.h"
#include <unordered_map>
#include "TC_LFObjectPool.h"
#include "MonitorServerDTO.h"
#include "CMonitorToolServer.h"

namespace procademy
{
	class CMonitorToolServer;

	class CMonitorServer : public CLF_LanServer
	{
	public:
		CMonitorServer();
		virtual ~CMonitorServer();
		bool	BeginServer();
		
		
	private:
		virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void OnClientJoin(SESSION_ID SessionID) override;
		virtual void OnClientLeave(SESSION_ID SessionID) override;
		virtual void OnRecv(SESSION_ID SessionID, CLanPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

		void				Init();
		bool				MonitorProc();
		bool				DBProc();
		void				LoadInitFile(const WCHAR* fileName);
		void				BeginThreads();
		void				WaitForThreadsFin();
		void				MakeMonitorStr(WCHAR* s, int size);
		void				ClearTPS();
		bool				JoinProc(SESSION_ID sessionID);
		bool				RecvProc(SESSION_ID sessionID, CLanPacket* packet);
		bool				LeaveProc(SESSION_ID sessionID);
		bool				ServerLoginProc(SESSION_ID sessionID, CLanPacket* packet);
		bool				UpdateDataProc(SESSION_ID sessionID, CLanPacket* packet);
		void				EnqueueDataProc(st_ServerClient* server, st_MonitorData* data);
		CNetPacket*			MakeMonitoringPacket(st_ServerClient* server, st_MonitorData* data);
		CNetPacket*			MakeMonitorLoginRes(BYTE Status);
		st_ServerClient*	FindServer(SESSION_ID sessionNo);
		void				InsertServer(SESSION_ID sessionNo, st_ServerClient* server);
		void				DeleteServer(SESSION_ID sessionNo);
		void				LockServer() { AcquireSRWLockExclusive(&mServerLock); }
		void				UnlockServer() { ReleaseSRWLockExclusive(&mServerLock); }

		static unsigned int WINAPI	MonitorThread(LPVOID arg);
		static unsigned int WINAPI	DBThread(LPVOID arg);

	private:
		std::unordered_map<u_int64, st_ServerClient*>		mServerClients;
		SRWLOCK												mServerLock;
		WCHAR												mDBIP[16];
		USHORT												mDBPort;
		HANDLE*												mThreads = nullptr;
		TC_LFObjectPool<st_MonitorData>						mMonitorDataPool;
		DWORD												mUpdateTPS = 0;
		CMonitorToolServer									mMonitorToolServer;
	};
}