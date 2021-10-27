#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFQueue.h"
#include "TC_LFObjectPool.h"

namespace procademy
{
	class CChatServerSingle : public CNetServerNoLock
	{
	private:
		struct st_Player
		{

		};

		struct st_MSG {
			BYTE			type;
			u_int64			sessionNo;
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

	private:
		static unsigned int WINAPI UpdateFunc(LPVOID arg);

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
		bool			CompleteMessage();
		bool			LoginProc(u_int64 sessionNo);
		bool			MoveSectorProc(u_int64 sessionNo);
		bool			SendMessageProc(u_int64 sessionNo);

		/// <summary>
		/// sessionNo�� player�� key ��
		/// </summary>
		/// <param name="sessionNo"></param>
		/// <returns></returns>
		st_Player*		FindPlayer(u_int64 sessionNo);
		void			InsertPlayer(st_Player* player);
		void			DeletePlayer(u_int64 sessionNo);

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
		HANDLE									mIOCP;

		TC_LFObjectPool<st_MSG>					mMsgPool;
		TC_LFQueue<st_MSG*>						mMsgQ;

		std::unordered_map<u_int64, st_Player*>	mPlayerMap;
		TC_LFObjectPool<st_Player>				mPlayerPool;

		std::list<st_Player*>						mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
	};
}
