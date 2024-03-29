#pragma once
#include "CNetServerNoLock.h"
#include <unordered_map>
#include "CDBConnector_TLS.h"
#include "CRedis_TLS.h"
#include "LoginServerDTO.h"
#include "CCpuUsage.h"
#include "TC_LFObjectPool.h"
#include <cpp_redis/cpp_redis>
#include "CMonitorClient.h"

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

namespace procademy
{
	struct st_Player;

	class CNetLoginServer : public CLF_NetServer
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
		bool			RunServer();
		void			RunningLoop();

	private:
		static unsigned int WINAPI MonitorFunc(LPVOID arg);
		static unsigned int WINAPI HeartbeatFunc(LPVOID arg);

		st_Player*		FindPlayer(SESSION_ID sessionNo);
		void			InsertPlayer(SESSION_ID sessionNo, st_Player* user);
		void			DeletePlayer(SESSION_ID sessionNo);
		void			BeginThreads();
		void			Init();
		void			LoadInitFile(const WCHAR* fileName);
		void			FreePlayer(st_Player* user);
		bool			JoinProc(SESSION_ID sessionNo);
		bool			LeaveProc(SESSION_ID sessionNo);
		bool			LoginProc(SESSION_ID sessionNo, CNetPacket* packet, WCHAR* msg);
		bool			CheckHeartProc();
		bool			MonitoringProc();
		void			MakeMonitorStr(WCHAR* s, int size);
		void			MakeTimeInfoStr(WCHAR* s, int size);
		void			ClearTPS();
		bool			TokenVerificationProc(INT64 accountNo, char* sessionKey, st_Player* output, 
			LARGE_INTEGER& beginDBTime, LARGE_INTEGER& beginRedisTime, LARGE_INTEGER& endRedisTime);
		void			UpdateTimeInfo(ULONGLONG loginBegin, ULONGLONG dbBegin, ULONGLONG redisBegin, ULONGLONG redisEnd, ULONGLONG loginEnd);
		void			LoginMonitorServer();
		void			SendMonitorDataProc();

		CNetPacket*		MakeCSResLogin(BYTE status, INT64 accountNo, const WCHAR* id, const WCHAR* nickName, BYTE index);
		CLanPacket*		MakeMonitorLogin(int serverNo);
		CLanPacket*		MakeMonitorPacket(BYTE dataType, int dataValue);


	private:
		enum {
			ERR_MSG_MAX = 256
		};

	private:
		std::unordered_map<SESSION_ID, st_Player*>	mPlayerMap;
		SRWLOCK										mPlayerMapLock;
		TC_LFObjectPool<st_Player>					mPlayerPool;
		CDBConnector*								mDBConnector;
		SRWLOCK										mDBConnectorLock;
		cpp_redis::client							mRedis;
		SRWLOCK										mRedisLock;
		int											mTimeOut;
		HANDLE										mhThreads[2];
		CCpuUsage									mCpuUsage;
		st_DummyServerInfo							mDummyServers[2];
		WCHAR										mTokenDBIP[16];
		USHORT										mTokenDBPort;
		WCHAR										mAccountDBIP[16];
		USHORT										mAccountDBPort;
		WCHAR										mAccountDBUser[32];
		WCHAR										mAccountDBPassword[32];
		WCHAR										mAccountDBSchema[32];
		bool										mbMonitoring;
		bool										mbTlsMode = false;

		alignas(64) DWORD							mLoginWaitCount = 0;
		DWORD										mLoginCount = 0;
		DWORD										mLoginTotal = 0;

		SRWLOCK										mTimeInfoLock;
		ULONGLONG									mLoginTimeSum = 0;
		ULONGLONG									mLoginTimeMax = 0;
		ULONGLONG									mLoginTimeMin = -1;
		ULONGLONG									mDBTimeSum = 0;
		ULONGLONG									mDBTimeMax = 0;
		ULONGLONG									mDBTimeMin = -1;
		ULONGLONG									mRedisTimeSum = 0;
		ULONGLONG									mRedisTimeMax = 0;
		ULONGLONG									mRedisTimeMin = -1;

		/// <summary>
		/// Monitor Client
		/// </summary>
		CMonitorClient								mMonitorClient;
		u_short										mMonitorPort = 0;
		WCHAR										mMonitorIP[32];
		int											mServerNo;
	};
}

