#pragma once

#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFObjectPool.h"
#include "ChatServerDTO.h"
#include "CCpuUsage.h"
#include <cpp_redis/cpp_redis>
#include "CSafeQueue.h"
#include "ObjectPool_TLS.h"
#include "CLanClient.h"
#include "CMonitorClient.h"

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

namespace procademy
{
	class CChatServerSingle : public CLF_NetServer
	{
		struct RatioMonitor
		{
			long joinCount;
			//alignas(64) long loginCount;
			long loginCount;
			//alignas(64) long leaveCount;
			long leaveCount;
			//alignas(64) long moveSectorCount;
			long moveSectorCount;
			//alignas(64) long sendMsgInCount;
			long sendMsgInCount;
			//alignas(64) long sendMsgOutCount;
			long sendMsgOutCount;
		};

	private:
		enum en_MSG_TYPE
		{
			MSG_TYPE_RECV,
			MSG_TYPE_JOIN,
			MSG_TYPE_LEAVE,
			MSG_TYPE_TIMEOUT,
			MSG_TYPE_VERIFICATION_SUCCESS,
			MSG_TYPE_VERIFICATION_FAIL
		};

	public:
		CChatServerSingle();
		virtual ~CChatServerSingle();
		virtual bool	OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void	OnClientJoin(SESSION_ID SessionID) override;
		virtual void	OnClientLeave(SESSION_ID SessionID) override;
		virtual void	OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void	OnError(int errorcode, const WCHAR* log) override;
		bool			BeginServer();
		void			WaitForThreadsFin();

	private:
		static unsigned int WINAPI UpdateFunc(LPVOID arg);
		static unsigned int WINAPI MonitorFunc(LPVOID arg);
		static unsigned int WINAPI HeartbeatFunc(LPVOID arg);
		static unsigned int WINAPI RedisFunc(LPVOID arg);

		/// <summary>
		/// OnRecv가 MsgQ에 넣을 때, 호출할 함수
		/// MsgQ에 패킷을 넣고, iocp에 Post 를 던져서 깨움
		/// </summary>
		/// <param name="packet">넣을 패킷</param>
		void			EnqueueMessage(st_MSG* packet);
		void			EnqueueRedisQ(SESSION_ID sessionNo, CNetPacket* packet);
		/// <summary>
		/// GQCS를 기준으로 반복적으로 메세지를 처리할 함수
		/// waittime을 둬서 주기적으로 Heartbeat 체크
		/// </summary>
		/// <returns></returns>
		void			GQCSProcEx();
		void			EventProc();
		/// <summary>
		/// 전 플레이어를 돌면서 플레이어 체크
		/// </summary>
		/// <returns></returns>
		bool			CheckHeartProc();
		bool			MonitoringProc();
		bool			CompleteMessage(SESSION_ID sessionNo, CNetPacket* packet);
		bool			JoinProc(SESSION_ID sessionNo);
		bool			LoginProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			LoginProc_Redis(SESSION_ID sessionNo, CNetPacket* packet);
		bool			LeaveProc(SESSION_ID sessionNo);
		bool			MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			HeartUpdateProc(SESSION_ID sessionNo);
		bool			CheckTimeOutProc();
		bool			CompleteLoginProc(SESSION_ID sessionNo, CNetPacket* packet, bool success);
		bool			RedisProc();
		void			BeginThreads();
		void			LoadInitFile(const WCHAR* fileName);
		void			FreePlayer(st_Player* player);

		/// <summary>
		/// sessionNo가 player의 key 값
		/// </summary>
		/// <param name="sessionNo"></param>
		/// <returns></returns>
		st_Player*		FindPlayer(SESSION_ID sessionNo);
		void			InsertPlayer(SESSION_ID sessionNo, st_Player* player);
		void			DeletePlayer(SESSION_ID sessionNo);
		void			Sector_AddPlayer(WORD x, WORD y, st_Player* player);
		void			Sector_RemovePlayer(WORD x, WORD y, st_Player* player);
		void			GetSectorAround(WORD x, WORD y, st_Sector_Around* output);
		DWORD			SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input);
		void			MakeMonitorStr(WCHAR* s, int size);
		void			MakeRatioMonitorStr(WCHAR* s, int size);
		void			PrintRecvSendRatio();
		void			ClearTPS();
		void			Init();
		void			RecordPerformence();
		void			LoginMonitorServer();
		void			SendMonitorDataProc();

		/// <summary>
		/// Make Packet Funcs
		/// </summary>
		CNetPacket* MakeCSResLogin(BYTE status, INT64 accountNo);
		CNetPacket* MakeCSResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY);
		CNetPacket* MakeCSResMessage(INT64 accountNo, WCHAR* ID, WCHAR* nickname, WORD messageLen, WCHAR* message);
		CNetPacket* MakeResultLogin(INT64 accountNo, WCHAR* ID, WCHAR* nickname);
		CLanPacket* MakeMonitorLogin(int serverNo);
		CLanPacket* MakeMonitorPacket(BYTE dataType, int dataValue);
	public:
		enum {
			SECTOR_MAX_Y = 100,
			SECTOR_MAX_X = 100
		};

	private:
		HANDLE									mUpdateThread;
		HANDLE									mMonitoringThread;
		HANDLE									mHeartbeatThread;
		HANDLE									mRedisThread;
		HANDLE									mIOCP;
		HANDLE									mRedisIOCP;
		cpp_redis::client						mRedis;
		WCHAR									mTokenDBIP[16];
		USHORT									mTokenDBPort;

		u_short									mMonitorPort = 0;
		WCHAR									mMonitorIP[32];
		CMonitorClient							mMonitorClient;

		ObjectPool_TLS<st_MSG>					mMsgPool;
		TC_LFQueue<st_MSG*>						mMsgQ;

		HANDLE									mUpdateEvent = INVALID_HANDLE_VALUE;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		TC_LFObjectPool<st_Player>				mPlayerPool;
		DWORD									mLoginCount = 0;
		DWORD									mUpdateTPS = 0;
		DWORD									mRedisTPS = 0;
		DWORD									mLoopCount = 0;
		DWORD									mGQCSCExNum = 0;

		st_Sector								mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
		int										mTimeOut;
		CCpuUsage								mCpuUsage;
		bool									mSendToWorker = true;
		bool									mGQCSEx;
		bool									mbMonitoring;
		bool									mbPrint;
		bool									mbRedisMode;
		bool									mbProfiler;
		int										mServerNo;
		RatioMonitor							mRatioMonitor;
	};
}
