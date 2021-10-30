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
			MSG_TYPE_JOIN,
			MSG_TYPE_LEAVE,
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
			SESSION_ID	sessionNo = 0;
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
		bool			BeginServer(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // ���� IP / ��Ʈ / ��Ŀ������ ��(������, ���׼�) / ���ۿɼ� / �ִ������� ��
		bool			BeginServer(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);

	private:
		static unsigned int WINAPI UpdateFunc(LPVOID arg);
		static unsigned int WINAPI MonitorFunc(LPVOID arg);
		static unsigned int WINAPI HeartbeatFunc(LPVOID arg);

		/// <summary>
		/// �ʱ� ����
		/// </summary>
		void			Initialize();
		/// <summary>
		/// OnRecv�� MsgQ�� ���� ��, ȣ���� �Լ�
		/// MsgQ�� ��Ŷ�� �ְ�, iocp�� Post �� ������ ����
		/// </summary>
		/// <param name="packet">���� ��Ŷ</param>
		void			EnqueueMessage(st_MSG* packet);
		/// <summary>
		/// GQCS�� �������� �ݺ������� �޼����� ó���� �Լ�
		/// waittime�� �ּ� �ֱ������� Heartbeat üũ
		/// </summary>
		/// <returns></returns>
		bool			GQCSProc();
		/// <summary>
		/// �� �÷��̾ ���鼭 �÷��̾� üũ
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

		/// <summary>
		/// sessionNo�� player�� key ��
		/// </summary>
		/// <param name="sessionNo"></param>
		/// <returns></returns>
		st_Player*		FindPlayer(SESSION_ID sessionNo);
		void			InsertPlayer(SESSION_ID sessionNo, st_Player* player);
		void			DeletePlayer(SESSION_ID sessionNo);
		void			Sector_AddPlayer(WORD x, WORD y, st_Player* player);
		void			Sector_RemovePlayer(WORD x, WORD y, st_Player* player);
		void			GetSectorAround(WORD x, WORD y, st_Sector_Around* output);
		void			SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input);

	/// <summary>
	/// Make Packet Funcs
	/// </summary>
		CNetPacket* MakeCSResLogin(BYTE status, SESSION_ID accountNo);
		CNetPacket* MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY);
		CNetPacket* MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message);

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
