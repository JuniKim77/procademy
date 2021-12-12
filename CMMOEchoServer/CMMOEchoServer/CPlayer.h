#pragma once
#include "CSession.h"

namespace procademy
{
	class CPlayer : public CSession
	{
	public:
		virtual void OnAuth_ClientJoin() override;
		virtual void OnAuth_ClientLeave() override;
		virtual void OnAuth_ClientRelease() override;
		virtual void OnAuth_Update() override;
		virtual void OnAuth_Packet(CNetPacket* packet) override;
		virtual void OnGame_ClientJoin() override;
		virtual void OnGame_ClientLeave() override;
		virtual void OnGame_ClientRelease() override;
		virtual void OnGame_Update() override;
		virtual void OnGame_Packet(CNetPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;

	private:

	};
}