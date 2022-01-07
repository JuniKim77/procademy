#include "CMonitorClient.h"
#include "CLanPacket.h"

procademy::CMonitorClient::CMonitorClient()
{
	Start();
}

procademy::CMonitorClient::~CMonitorClient()
{
}

bool procademy::CMonitorClient::BeginClient()
{
	return Start();
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
