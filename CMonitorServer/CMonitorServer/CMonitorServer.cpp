#include "CMonitorServer.h"

bool procademy::CMonitorServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return false;
}

void procademy::CMonitorServer::OnClientJoin(SESSION_ID SessionID)
{
}

void procademy::CMonitorServer::OnClientLeave(SESSION_ID SessionID)
{
}

void procademy::CMonitorServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
}

void procademy::CMonitorServer::OnError(int errorcode, const WCHAR* log)
{
}
