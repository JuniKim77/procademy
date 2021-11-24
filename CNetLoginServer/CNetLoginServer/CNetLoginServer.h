#pragma once
#include "CNetServerNoLock.h"
#include <unordered_map>
#include "CDBConnector_TLS.h"
#include "CCpuUsage.h"
#include "TC_LFObjectPool.h"

namespace procademy
{
	struct st_User;

	class CNetLoginServer : public CNetServerNoLock
	{
	public:
		CNetLoginServer();
		virtual ~CNetLoginServer();
		virtual bool	OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void	OnClientJoin(SESSION_ID SessionID) override;
		virtual void	OnClientLeave(SESSION_ID SessionID) override;
		virtual void	OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void	OnError(int errorcode, const WCHAR* log) override;
		bool			BeginServer();
		void			WaitForThreadsFin();

	private:
		static unsigned int WINAPI MonitorFunc(LPVOID arg);
		static unsigned int WINAPI HeartbeatFunc(LPVOID arg);

		st_User*		FindUser(SESSION_ID sessionNo);
		void			InsertUser(SESSION_ID sessionNo, st_User* user);
		void			DeleteUser(SESSION_ID sessionNo);
		void			BeginThreads();
		void			Init();
		void			LoadInitFile(const WCHAR* fileName);
		void			FreeUser(st_User* user);
		bool			JoinUserProc(SESSION_ID sessionNo);
		bool			LeaveUserProc(SESSION_ID sessionNo);
		bool			LoginProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			CheckHeartProc();
		bool			MonitoringProc();
		void			MakeMonitorStr(WCHAR* s, int size);
		void			ClearTPS();
		bool			ReqAccountDB(INT64 accountNo, st_User* output);

		CNetPacket*		MakeCSResLogin(BYTE status, INT64 accountNo);

	private:
		std::unordered_map<SESSION_ID, st_User*>	mUserMap;
		SRWLOCK										mUserMapLock;
		TC_LFObjectPool<st_User>					mUserPool;
		CDBConnector*								mDBConnector;
		SRWLOCK										mDBConnectorLock;
		int											mTimeOut;
		HANDLE										mhThreads[2];
		CCpuUsage									mCpuUsage;
	};
}

