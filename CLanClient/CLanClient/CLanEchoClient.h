#pragma once
#include "CLanClient.h"

namespace procademy
{
	class CLanEchoClient : public CLanClient
	{
	public:
		CLanEchoClient();
		virtual ~CLanEchoClient();


	private:
		virtual void OnEnterJoinServer() override;
		virtual void OnLeaveServer() override;
		virtual void OnRecv(CLanPacket*) override;
		virtual void OnSend(int sendsize) override;
		virtual void OnError(int errorcode, const WCHAR*) override;

	private:

	};
}
