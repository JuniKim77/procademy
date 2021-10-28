#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFObjectPool.h"

namespace procademy
{
	class CChatServerSingle : public CNetServerNoLock
	{
	private:
		enum en_MSG_TYPE
		{
			MSG_TYPE_RECV,
			MSG_TYPE_CONNECT,
			MSG_TYPE_DISCONNECT,
			MSG_TYPE_TIMEOUT
		};

		struct st_Sector
		{
			int x;
			int y;
		};

		struct st_Sector_Around
		{
			int count;
			st_Sector around[9];
		};

		struct st_Player
		{
			ULONGLONG	lastRecvTime = 0;
			INT64		accountNo = 0;
			WCHAR		ID[20];
			WCHAR		nickName[20];
			short		curSectorX = -1;
			short		curSectorY = -1;
			bool		bLogin = false;
		};

		struct st_MSG {
			BYTE			type;
			SESSION_ID		sessionNo;
			CNetPacket*		packet;
		};

	public:
		CChatServerSingle();
		virtual ~CChatServerSingle();
		virtual bool	OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void	OnClientJoin(SESSION_ID SessionID) override;
		virtual void	OnClientLeave(SESSION_ID SessionID) override;
		virtual void	OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void	OnError(int errorcode, const WCHAR* log) override;
		bool			BeginServer(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
		bool			BeginServer(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);

	private:
		static unsigned int WINAPI UpdateFunc(LPVOID arg);
		static unsigned int WINAPI MonitorFunc(LPVOID arg);
		static unsigned int WINAPI HeartbeatFunc(LPVOID arg);

		/// <summary>
		/// 초기 설정
		/// </summary>
		void			Initialize();
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
		bool			GQCSProc();
		/// <summary>
		/// 전 플레이어를 돌면서 플레이어 체크
		/// </summary>
		/// <returns></returns>
		bool			CheckHeart();
		bool			MonitoringProc();
		bool			CompleteMessage(u_int64 sessionNo, CNetPacket* packet);
		bool			LoginProc(u_int64 sessionNo, CNetPacket* packet);
		bool			MoveSectorProc(u_int64 sessionNo, CNetPacket* packet);
		bool			SendMessageProc(u_int64 sessionNo, CNetPacket* packet);

		/// <summary>
		/// sessionNo가 player의 key 값
		/// </summary>
		/// <param name="sessionNo"></param>
		/// <returns></returns>
		st_Player*		FindPlayer(u_int64 sessionNo);
		void			InsertPlayer(u_int64 sessionNo, st_Player* player);
		void			DeletePlayer(u_int64 sessionNo);
		void			Sector_AddPlayer(WORD x, WORD y, st_Player* player);
		void			Sector_RemovePlayer(WORD x, WORD y, st_Player* player);
		void			GetSectorAround(WORD x, WORD y, st_Sector_Around* output);
		void			SendMessageSectorAround(CNetPacket* packet, WORD size, st_Sector_Around* input);

	/// <summary>
	/// Make Packet Funcs
	/// </summary>
		CNetPacket* MakeCSResLogin(BYTE status, INT64 accountNo);
		CNetPacket* MakeCSResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY);
		CNetPacket* MakeCSResMessage(INT64 accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message);

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
		TC_LFQueue<st_MSG*>						mMsgQ;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		TC_LFObjectPool<st_Player>				mPlayerPool;

		std::list<st_Player*>					mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
	};
}
