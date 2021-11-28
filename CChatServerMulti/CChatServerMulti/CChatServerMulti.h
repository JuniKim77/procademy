#pragma once

#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFObjectPool.h"
#include "ChatServerDTO.h"
#include "CCpuUsage.h"

namespace procademy
{
	class CChatServerMulti : public CNetServerNoLock
	{
	public:
		CChatServerMulti();
		virtual ~CChatServerMulti();

		virtual bool				OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void				OnClientJoin(SESSION_ID SessionID) override;
		virtual void				OnClientLeave(SESSION_ID SessionID) override;
		virtual void				OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void				OnError(int errorcode, const WCHAR* log) override;
		bool						BeginServer();
		void						WaitForThreadsFin();

	private:
		void						Init();
		bool						CheckHeart();
		bool						MonitoringProc();
		bool						JoinProc(SESSION_ID sessionNo);
		bool						LoginProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						LeaveProc(SESSION_ID sessionNo);
		bool						MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool						HeartUpdateProc(SESSION_ID sessionNo);
		bool						CheckTimeOutProc();
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
		void						PrintRecvSendRatio();
		void						ClearTPS();

		CNetPacket*					MakeCSResLogin(BYTE status, SESSION_ID accountNo);
		CNetPacket*					MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY);
		CNetPacket*					MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message);

		static unsigned int WINAPI	MonitorFunc(LPVOID arg);
		static unsigned int WINAPI	HeartbeatFunc(LPVOID arg);

	public:
		enum {
			SECTOR_MAX_Y = 100,
			SECTOR_MAX_X = 100
		};

	private:
		HANDLE									mMonitoringThread;
		HANDLE									mHeartbeatThread;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		SRWLOCK									mPlayerMapLock;
		TC_LFObjectPool<st_Player>				mPlayerPool;
		DWORD									mLoginCount = 0;
		DWORD									mUpdateTPS = 0;
		st_Sector								mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
		SRWLOCK									mSectorLock;
		int										mTimeOut;
		CCpuUsage								mCpuUsage;
		bool									mbMonitoring = true;
		bool									mbPrint = false;
	};
}