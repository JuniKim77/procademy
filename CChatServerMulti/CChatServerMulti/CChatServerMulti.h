#pragma once

#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFObjectPool.h"
#include "MultiChatServerDTO.h"
#include "CCpuUsage.h"
#include <cpp_redis/cpp_redis>

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

namespace procademy
{
	class CChatServerMulti : public CLF_NetServer
	{
		struct RatioMonitor
		{
			long			 joinCount;
			alignas(64) long loginCount;
			alignas(64) long leaveCount;
			alignas(64) long moveSectorCount;
			alignas(64) long sendMsgInCount;
			alignas(64) long sendMsgOutCount;
		};
	public:
		CChatServerMulti();
		virtual ~CChatServerMulti();

		virtual bool				OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void				OnClientJoin(SESSION_ID SessionID) override;
		virtual void				OnClientLeave(SESSION_ID SessionID) override;
		virtual void				OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void				OnError(int errorcode, const WCHAR* log) override;
		bool						BeginServer();
		bool						RunServer();
		void						RunningLoop();

	private:
		void						Init();
		bool						CheckHeartProc();
		bool						MonitoringProc();
		bool						JoinProc(SESSION_ID sessionNo);
		bool						RecvProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						LoginProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						LeaveProc(SESSION_ID sessionNo);
		bool						MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						HeartUpdateProc(SESSION_ID sessionNo);
		void						BeginThreads();
		void						LoadInitFile(const WCHAR* fileName);
		void						FreePlayer(st_Player* player);
		st_Player*					FindPlayer(SESSION_ID sessionNo);
		void						InsertPlayer(SESSION_ID sessionNo, st_Player* player);
		void						DeletePlayer(SESSION_ID sessionNo);
		void						Sector_AddPlayer(WORD x, WORD y, st_Player* player);
		void						Sector_RemovePlayer(WORD x, WORD y, st_Player* player);
		void						GetSectorAround(WORD x, WORD y, st_Sector_Around* output);
		DWORD						SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input);
		void						MakeMonitorStr(WCHAR* s, int size);
		void						MakeRatioMonitorStr(WCHAR* s, int size);
		void						ClearTPS();
		void						RecordPerformentce();
		void						LockSector(WORD x, WORD y, bool exclusive = true);
		void						UnlockSector(WORD x, WORD y, bool exclusive = true);
		void						LockSectors(WORD x1, WORD y1, WORD x2, WORD y2, bool exclusive = true);
		void						UnlockSectors(WORD x1, WORD y1, WORD x2, WORD y2, bool exclusive = true);
		void						LockPlayerMap(bool exclusive = true);
		void						UnlockPlayerMap(bool exclusive = true);

		CNetPacket*					MakeCSResLogin(BYTE status, SESSION_ID accountNo);
		CNetPacket*					MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY);
		CNetPacket*					MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message);

		static unsigned int WINAPI	MonitorFunc(LPVOID arg);
		static unsigned int WINAPI	HeartbeatFunc(LPVOID arg);

	public:
		enum {
			SECTOR_MAX_Y = 50,
			SECTOR_MAX_X = 50
		};

	private:
		HANDLE									mMonitoringThread;
		HANDLE									mHeartbeatThread;
		cpp_redis::client						mRedis;
		WCHAR									mTokenDBIP[16];
		USHORT									mTokenDBPort;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		SRWLOCK									mPlayerMapLock;
		TC_LFObjectPool<st_Player>				mPlayerPool;
		alignas(64) DWORD						mUpdateTPS = 0;
		st_Sector								mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
		/// <summary>
		/// N X N 개의 섹터 동기화 객체
		/// </summary>
		int										mSectorLockIndex[SECTOR_MAX_Y][SECTOR_MAX_X];
		int										mTimeOut;
		CCpuUsage								mCpuUsage;
		bool									mbMonitoring = true;
		bool									mbPrint = false;
		bool									mbRedisMode = false;
		RatioMonitor							mRatioMonitor;
	};
}