#include "CMonitorClient.h"
#include "CLanPacket.h"

procademy::CMonitorClient::CMonitorClient()
{
}

procademy::CMonitorClient::~CMonitorClient()
{
}

bool procademy::CMonitorClient::BeginClient()
{
	Begin();

	return true;
}

void procademy::CMonitorClient::RunClient()
{
	Start();
}

void procademy::CMonitorClient::OnEnterJoinServer()
{
	mbJoin = true;
}

void procademy::CMonitorClient::OnLeaveServer()
{
	mbJoin = false;
	mbLogin = false;
}

void procademy::CMonitorClient::OnRecv(CLanPacket* packet)
{
}

void procademy::CMonitorClient::OnSend(int sendsize)
{
}

void procademy::CMonitorClient::OnError(int errorcode, const WCHAR*)
{
}
