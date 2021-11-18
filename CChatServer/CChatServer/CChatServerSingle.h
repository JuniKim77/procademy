#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFObjectPool.h"
#include "ChatServerDTO.h"
#include "CCpuUsage.h"

namespace procademy
{
	class CChatServerSingle : public CNetServerNoLock
	{
	private:
		enum en_MSG_TYPE
		{
			MSG_TYPE_RECV,
			MSG_TYPE_JOIN,
			MSG_TYPE_LEAVE,
			MSG_TYPE_TIMEOUT
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

		/// <summary>
		/// OnRecv가 MsgQ에 넣을 때, 호출할 함수
		/// MsgQ에 패킷을 넣고, iocp에 Post 를 던져서 깨움
		/// </summary>
		/// <param name="packet">넣을 패킷</param>
		void			EnqueueMessage(st_MSG* packet);
		/// <summary>
		/// GQCS를 기준으로 반복적으로 메세지를 처리할 함수
		/// waittime을 둬서 주기적으로 Heartbeat 체크
		/// </summary>
		/// <returns></returns>
		void			GQCSProc();
		void			GQCSProcEx();
		/// <summary>
		/// 전 플레이어를 돌면서 플레이어 체크
		/// </summary>
		/// <returns></returns>
		bool			CheckHeart();
		bool			MonitoringProc();
		bool			CompleteMessage(SESSION_ID sessionNo, CNetPacket* packet);
		bool			JoinProc(SESSION_ID sessionNo);
		bool			LoginProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			LeaveProc(SESSION_ID sessionNo);
		bool			MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet);
		bool			HeartUpdateProc(SESSION_ID sessionNo);
		bool			CheckTimeOutProc();
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
		void			PrintRecvSendRatio();
		void			ClearTPS();

	/// <summary>
	/// Make Packet Funcs
	/// </summary>
		CNetPacket*		MakeCSResLogin(BYTE status, SESSION_ID accountNo);
		CNetPacket*		MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY);
		CNetPacket*		MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message);

	public:
		enum {
			SECTOR_MAX_Y = 100,
			SECTOR_MAX_X = 100
		};

	private:
		HANDLE									mUpdateThread;
		HANDLE									mMonitoringThread;
		HANDLE									mHeartbeatThread;
		HANDLE									mIOCP;

		TC_LFObjectPool<st_MSG>					mMsgPool;
		//TC_LFQueue<st_MSG*>						mMsgQ;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		//std::unordered_map<u_int64, int>		mLeavedMap;
		TC_LFObjectPool<st_Player>				mPlayerPool;
		DWORD									mLoginCount = 0;
		DWORD									mUpdateTPS = 0;
		DWORD									mGQCSCount = 0;
		DWORD									mGQCSCExNum = 0;

		st_Sector								mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
		int										mTimeOut;
		CCpuUsage								mCpuUsage;
		bool									mSendToWorker = true;
		bool									mGQCSEx;
};
}
