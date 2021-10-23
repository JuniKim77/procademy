#include "CNetServerNoLock.h"
#include <unordered_map>
#include <list>
#include "TC_LFQueue.h"
#include "TC_LFObjectPool.h"

namespace procademy
{
	class CPlayer;

	class CChatServerSingle : public CNetServerNoLock
	{
	private:
		struct st_MSG {
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
		static unsigned int WINAPI CheckHeartFunc(LPVOID arg);

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
		/// waittime�� Infinite�� �ؼ� �޼����� ���� ���� �۵���
		/// </summary>
		/// <returns></returns>
		bool			CompleteMessage();
		/// <summary>
		/// �� �÷��̾ ���鼭 �÷��̾� üũ
		/// </summary>
		/// <returns></returns>
		bool			CheckHeart();
		bool			LoginProc(u_int64 sessionNo);
		bool			MoveSectorProc(u_int64 sessionNo);
		bool			SendMessageProc(u_int64 sessionNo);

		/// <summary>
		/// sessionNo�� player�� key ��
		/// </summary>
		/// <param name="sessionNo"></param>
		/// <returns></returns>
		CPlayer*		SearchPlayer(u_int64 sessionNo);
		void			InsertPlayer(CPlayer* player);
		void			DeletePlayer(u_int64 sessionNo);
		void			LockPlayerMap();
		void			UnlockPlayerMap();
		void			LockField();
		void			UnlockField();

	public:
		enum {
			SECTOR_MAX_Y = 100,
			SECTOR_MAX_X = 100
		};

	private:
		HANDLE									mUpdateThread;
		HANDLE									mHeartCheckThread;
		HANDLE									mIOCP;

		TC_LFQueue<st_MSG*>						mMsgQ;

		std::unordered_map<u_int64, CPlayer*>	mPlayerMap;
		SRWLOCK									mPlayerMapLock;
		TC_LFObjectPool<CPlayer>				mPlayerPool;

		std::list<CPlayer*>						mSector[SECTOR_MAX_Y][SECTOR_MAX_X];
		SRWLOCK									mFieldLock;
	};
}
