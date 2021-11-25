#pragma once
#include "CNetServerNoLock.h"
#include <unordered_map>
#include "CDBConnector_TLS.h"
#include "LoginServerDTO.h"
#include "CCpuUsage.h"
#include "TC_LFObjectPool.h"
#include <cpp_redis/cpp_redis>

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

namespace procademy
{
	struct st_Player;

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
		void			ClearTPS();
		bool			TokenVerificationProc(INT64 accountNo, char* sessionKey, st_Player* output);

		CNetPacket*		MakeCSResLogin(BYTE status, INT64 accountNo, const WCHAR* id, const WCHAR* nickName);

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
		int											mTimeOut;
		HANDLE										mhThreads[2];
		CCpuUsage									mCpuUsage;
		WCHAR										mGameServerIP[16];
		USHORT										mGameServerPort;
		WCHAR										mChatServerIP[16];
		USHORT										mChatServerPort;
		WCHAR										mTokenDBIP[16];
		USHORT										mTokenDBPort;
		WCHAR										mAccountDBIP[16];
		USHORT										mAccountDBPort;
		WCHAR										mAccountDBUser[32];
		WCHAR										mAccountDBPassword[32];
		WCHAR										mAccountDBSchema[32];
		bool										mbMonitoring;
		DWORD										mLoginCount = 0;
	};
}

