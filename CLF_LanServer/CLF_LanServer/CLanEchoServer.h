#pragma once
#include "CLF_LanServer.h"
#include <unordered_map>

namespace procademy
{
	class CLanEchoServer : public CLF_LanServer
	{
	public:
		CLanEchoServer();
		virtual ~CLanEchoServer();
		bool			BeginServer();
		

	private:
		static unsigned int WINAPI	MonitorFunc(LPVOID arg);

		virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void OnClientJoin(SESSION_ID SessionID) override;
		virtual void OnClientLeave(SESSION_ID SessionID) override;
		virtual void OnRecv(SESSION_ID SessionID, CLanPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

		void			Init();
		void			WaitForThreadsFin();
		bool			MonitoringProc();
		void			InsertSessionID(u_int64 sessionNo);
		void			DeleteSessionID(u_int64 sessionNo);
		void			LockMap() { AcquireSRWLockExclusive(&mSessionLock); }
		void			UnlockMap() { ReleaseSRWLockExclusive(&mSessionLock); }
		void			MakeMonitorStr(WCHAR* s, int size);
		void			BeginThreads();

	private:
		std::unordered_map<u_int64, int>	mSessionJoinMap;
		SRWLOCK								mSessionLock;
		HANDLE								mMonitoringThread = INVALID_HANDLE_VALUE;
	};
}