#pragma once
#include "CSession.h"

namespace procademy
{
	class CMMOEchoServer;

	class CPlayer : public CSession
	{
	public:
		virtual void OnAuth_ClientJoin() override;
		virtual void OnAuth_ClientLeave() override;
		virtual void OnAuth_ClientRelease() override;
		virtual void OnAuth_Packet(CNetPacket* packet) override;
		virtual void OnGame_ClientJoin() override;
		virtual void OnGame_ClientLeave() override;
		virtual void OnGame_ClientRelease() override;
		virtual void OnGame_Packet(CNetPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

		void SetServer(CMMOEchoServer* server) { mServer = server; }

	private:
		bool			LoginProc(CNetPacket* packet);
		bool			EchoProc(CNetPacket* packet);

		CNetPacket*		MakeCSGameResLogin(BYTE status, INT64 accountNo);
		CNetPacket*		MakeCSGameResEcho(INT64 accountNo, LONGLONG sendTick);

	private:
		INT64					mAccountNo = 0;
		bool					mbLogin = false;
		CMMOEchoServer*			mServer = nullptr;
	};
}