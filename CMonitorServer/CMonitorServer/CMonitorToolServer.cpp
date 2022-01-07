#include "CMonitorToolServer.h"
#include "CLogger.h"
#include "MonitorProtocol.h"
#include "CNetPacket.h"
#include "TextParser.h"

procademy::CMonitorToolServer::CMonitorToolServer()
{
	LoadInitFile(L"Server.cnf");
	Init();
}

procademy::CMonitorToolServer::~CMonitorToolServer()
{
}

bool procademy::CMonitorToolServer::BeginServer()
{
	if (Start() == false)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

		return false;
	}

	return true;
}

void procademy::CMonitorToolServer::QuitMonitorServer()
{
	QuitServer();
}

void procademy::CMonitorToolServer::StopMonitorServer()
{
	Stop();
}

void procademy::CMonitorToolServer::StartMonitorServer()
{
	Start();
}

void procademy::CMonitorToolServer::SendDataToAllClinet(CNetPacket* packet)
{
	LockServer();

	for (auto iter = mMonitorClients.begin(); iter != mMonitorClients.end(); ++iter)
	{
		if (iter->second->bLogin)
		{
			SendPacket(iter->second->sessionNo, packet);
		}
	}

	UnlockServer();
}

bool procademy::CMonitorToolServer::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void procademy::CMonitorToolServer::OnClientJoin(SESSION_ID SessionID)
{
	JoinProc(SessionID);
}

void procademy::CMonitorToolServer::OnClientLeave(SESSION_ID SessionID)
{
	LeaveProc(SessionID);
}

void procademy::CMonitorToolServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
	RecvProc(SessionID, packet);
}

void procademy::CMonitorToolServer::OnError(int errorcode, const WCHAR* log)
{
}

void procademy::CMonitorToolServer::Init()
{
	InitializeSRWLock(&mToolServerLock);
}

bool procademy::CMonitorToolServer::JoinProc(SESSION_ID sessionID)
{
	st_MonitorClient* monitor = FindMonitorTool(sessionID);

	if (monitor != nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Monitor[%llu]", sessionID);

		CRASH();
		return false;
	}

	monitor = new st_MonitorClient;
	monitor->sessionNo = sessionID;

	LockServer();
	InsertMonitorTool(sessionID, monitor);
	UnlockServer();

	return false;
}

bool procademy::CMonitorToolServer::RecvProc(SESSION_ID sessionID, CNetPacket* packet)
{
	WORD type;
	bool ret = false;

	*packet >> type;

	switch (type)
	{
	case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
		MonitorLoginProc(sessionID, packet);
		break;
	default:
		CRASH();
		break;
	}

	return true;
}

bool procademy::CMonitorToolServer::LeaveProc(SESSION_ID sessionID)
{
	LockServer();

	st_MonitorClient* monitor = FindMonitorTool(sessionID);

	if (monitor == nullptr)
	{
		UnlockServer();
		CRASH();

		return false;
	}

	DeleteMonitorTool(sessionID);

	delete monitor;

	UnlockServer();

	return true;
}

bool procademy::CMonitorToolServer::MonitorLoginProc(SESSION_ID sessionID, CNetPacket* packet)
{
	char	LoginSessionKey[33];

	packet->GetData(LoginSessionKey, 32);
	LoginSessionKey[32] = '\0';

	int test = strcmp(LoginSessionKey, mLoginSessionKey);

	if (strcmp(LoginSessionKey, mLoginSessionKey) != 0)
	{
		return false;
	}

	LockServer();

	st_MonitorClient* monitor = FindMonitorTool(sessionID);

	if (monitor == nullptr)
	{
		UnlockServer();
		CRASH();

		return false;
	}

	monitor->bLogin = true;

	UnlockServer();

	return true;
}

void procademy::CMonitorToolServer::LoadInitFile(const WCHAR* fileName)
{
	TextParser  tp;
	int         num;
	WCHAR       buffer[MAX_PARSER_LENGTH] = { 0, };

	tp.LoadFile(fileName);

	tp.GetValue(L"LOGIN_KEY", buffer);
	WideCharToMultiByte(CP_ACP, 0, buffer, -1, mLoginSessionKey, sizeof(mLoginSessionKey), NULL, NULL);
}

procademy::st_MonitorClient* procademy::CMonitorToolServer::FindMonitorTool(SESSION_ID sessionNo)
{
	std::unordered_map<u_int64, st_MonitorClient*>::iterator iter = mMonitorClients.find(sessionNo);

	if (iter == mMonitorClients.end())
	{
		return nullptr;
	}

	return iter->second;
}

void procademy::CMonitorToolServer::InsertMonitorTool(SESSION_ID sessionNo, st_MonitorClient* monitor)
{
	mMonitorClients[sessionNo] = monitor;
}

void procademy::CMonitorToolServer::DeleteMonitorTool(SESSION_ID sessionNo)
{
	mMonitorClients.erase(sessionNo);
}
