#pragma once
#include "CLanClient.h"

namespace procademy
{
	class CMonitorClient : public CLanClient
	{
	public:
		CMonitorClient();
		virtual ~CMonitorClient();
		bool	BeginClient();
		bool	IsLogin() { return mbLogin; }
		bool	IsJoin() { return mbJoin; }
		void	SetLogin() { mbLogin = true; }

	private:
		virtual void OnEnterJoinServer() override;
		virtual void OnLeaveServer() override;
		virtual void OnRecv(CLanPacket* packet) override;
		virtual void OnSend(int sendsize) override;
		virtual void OnError(int errorcode, const WCHAR*) override;

	private:
		bool mbJoin = false;
		bool mbLogin = false;
	};
}