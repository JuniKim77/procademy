#include "CPlayer.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CMMOEchoServer.h"

void procademy::CPlayer::OnAuth_ClientJoin()
{
}

void procademy::CPlayer::OnAuth_ClientLeave()
{
}

void procademy::CPlayer::OnAuth_ClientRelease()
{
	if (mbLogin)
	{
		InterlockedDecrement(&mServer->mLoginCount);
	}

	mAccountNo = 0;
	mbLogin = false;
}

void procademy::CPlayer::OnAuth_Packet(CNetPacket* packet)
{
	WORD type;
	bool ret = false;

	*packet >> type;

	switch (type)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:
		ret = LoginProc(packet);
		break;
	default:
		CRASH();
		break;
	}

	if (ret == false)
		CRASH();
}

void procademy::CPlayer::OnGame_ClientJoin()
{
}

void procademy::CPlayer::OnGame_ClientLeave()
{
}

void procademy::CPlayer::OnGame_ClientRelease()
{
	mAccountNo = 0;
	mbLogin = false;

	InterlockedDecrement(&mServer->mLoginCount);
}

void procademy::CPlayer::OnGame_Packet(CNetPacket* packet)
{
	WORD type;
	bool ret = false;

	*packet >> type;

	switch (type)
	{
	case en_PACKET_CS_GAME_REQ_ECHO:
		ret = EchoProc(packet);
		break;
	default:
		
		break;
	}

	if (ret == false)
		CRASH();
}

void procademy::CPlayer::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CPlayer::LoginProc(CNetPacket* packet)
{
	INT64	    AccountNo;
	char	    SessionKey[64];		// 인증토큰
	int			version;

	*packet >> AccountNo;
	packet->GetData(SessionKey, 64);
	*packet >> version;

	mAccountNo = AccountNo;
	mbLogin = true;

	CNetPacket* res = MakeCSGameResLogin(1, AccountNo);
	{
		SendPacket(res);
	}
	res->SubRef();

	InterlockedIncrement(&mServer->mLoginCount);

	toGame = true;

	return true;
}

bool procademy::CPlayer::EchoProc(CNetPacket* packet)
{
	INT64		AccountNo;
	LONGLONG	SendTick;

	*packet >> AccountNo >> SendTick;

	if (AccountNo != mAccountNo || mbLogin == false)
	{
		return false;
	}

	CNetPacket* res = MakeCSGameResEcho(AccountNo, SendTick);
	{
		SendPacket(res);
	}
	res->SubRef();

	return true;
}

procademy::CNetPacket* procademy::CPlayer::MakeCSGameResLogin(BYTE status, INT64 accountNo)
{
	CNetPacket* packet = CNetPacket::AllocAddRef();

	*packet << (WORD)en_PACKET_CS_GAME_RES_LOGIN << status << accountNo;

	packet->SetHeader(false);
	packet->Encode();

	return packet;
}

procademy::CNetPacket* procademy::CPlayer::MakeCSGameResEcho(INT64 accountNo, LONGLONG sendTick)
{
	CNetPacket* packet = CNetPacket::AllocAddRef();

	*packet << (WORD)en_PACKET_CS_GAME_RES_ECHO << accountNo << sendTick;

	packet->SetHeader(false);
	packet->Encode();

	return packet;
}
