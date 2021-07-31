#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ActionDefine.h"
#include <stdio.h>
#include "NetworkProcs.h"
#include <unordered_map>
#include "Session.h"
#include <vector>
#include "CPacket.h"
#include "User.h"
#include "Container.h"
#include "CPacket.h"
#include "Sector.h"
#include "PacketCreater.h"
#include "MyProfiler.h"
#include "PacketDefine.h"

using namespace std;

SOCKET g_listenSocket;
DWORD g_SessionNo = 1;
unordered_map<DWORD, Session*> g_sessions;
unordered_map<DWORD, User*> g_users;
extern CLogger g_Logger;
extern std::list<User*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void CreateServer()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		exit(1);
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(dfNETWORK_PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	g_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_listenSocket == INVALID_SOCKET)
	{
		LogError(L"SOCKET ERROR : Socket Create Error", g_listenSocket);

		exit(1);
	}

	if (bind(g_listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LogError(L"SOCKET ERROR : Binding Error", g_listenSocket);

		exit(1);
	}

	u_long on = 1;
	if (ioctlsocket(g_listenSocket, FIONBIO, &on) == SOCKET_ERROR)
	{
		LogError(L"SOCKET ERROR : NonBlocked Socket Error", g_listenSocket);

		exit(1);
	}

	if (listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		LogError(L"SOCKET ERROR : Listening Error", g_listenSocket);

		exit(1);
	}

	g_Logger._Log(dfLOG_LEVEL_NOTICE, L"Server begin... [Port: %d]\n", dfNETWORK_PORT);
}

void NetWorkProc()
{
	FD_SET rset;
	FD_SET wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	DWORD sessionKeyTable[FD_SETSIZE] = { 0, };
	int count = 0;

	FD_SET(g_listenSocket, &rset);
	sessionKeyTable[0] = (DWORD)g_listenSocket;

	count++;

	for (auto iter = g_sessions.begin(); iter != g_sessions.end();)
	{
		Session* session = iter->second;
		iter++; // 미리 증가

		if (session == nullptr)
			continue;

		sessionKeyTable[count] = session->mSessionNo;
		FD_SET(session->mSocket, &rset);

		if (session->mSendBuffer.GetUseSize() > 0)
		{
			FD_SET(session->mSocket, &wset);
		}

		count++;

		if (count == FD_SETSIZE)
		{
			SelectProc(sessionKeyTable, &rset, &wset);

			FD_ZERO(&rset);
			FD_ZERO(&wset);
			memset(sessionKeyTable, 0, sizeof(sessionKeyTable));

			FD_SET(g_listenSocket, &rset);
			sessionKeyTable[0] = (DWORD)g_listenSocket;

			count = 1;
		}
	}
	
	SelectProc(sessionKeyTable, &rset, &wset);
}

void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset)
{
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	int numSelected = select(0, rset, wset, NULL, &tval);

	if (numSelected == SOCKET_ERROR)
	{
		LogError(L"Select 에러", g_listenSocket);

		exit(1);
	}

	if (FD_ISSET(keyTable[0], rset))
	{
		AcceptProc();
		--numSelected;
	}

	for (int i = 1; i < FD_SETSIZE && numSelected > 0; ++i)
	{
		Session* session = FindSession(keyTable[i]);

		if (session == nullptr)
			continue;

		if (FD_ISSET(session->mSocket, wset))
		{
			ProfileBegin(L"WriteProc");
			session->writeProc();
			ProfileEnd(L"WriteProc");
			--numSelected;
		}

		if (FD_ISSET(session->mSocket, rset))
		{
			session->receiveProc();
			--numSelected;
		}

		// DisconnectProc
		if (session->mbAlive == false)
		{
			DisconnectProc(session->mSessionNo);
		}
	}
}

void DisconnectProc(DWORD sessionKey)
{
	User* user = FindUser(sessionKey);

	// 퇴장 로직 처리
	CPacket packet;
	cpSC_DeleteUser(&packet, user->userNo);
	SendPacket_Around(user->userNo, &packet, true);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Disconnect [UserNo: %d]\n", user->userNo);

	Sector_RemoveUser(user, true);
	DeleteSessionData(sessionKey);
	DeleteUserData(sessionKey);
}

void AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int len = sizeof(clientAddr);

	SOCKET client = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &len);

	if (client == INVALID_SOCKET)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		LogError(L"Accept 에러", g_listenSocket);

		exit(1);
	}

	WCHAR temp[16] = { 0, };
	InetNtop(AF_INET, &clientAddr.sin_addr, temp, 16);

	g_Logger._Log(dfLOG_LEVEL_NOTICE, L"Accept: IP - %s, Port - %d [UserNo: %d]\n",
		temp, ntohs(clientAddr.sin_port), g_SessionNo);

	u_long on = 1;
	if (ioctlsocket(client, FIONBIO, &on) == SOCKET_ERROR) {
		LogError(L"NonBlocked Socket Error", client);

		exit(1);
	}

	Session* session = new Session(client, ntohs(clientAddr.sin_port),
		clientAddr.sin_addr.S_un.S_addr, g_SessionNo);

	// 껍데기 유저
	User* user = new User;
	user->hp = 100;
	int x = rand() % dfRANGE_MOVE_RIGHT;
	int y = rand() % dfRANGE_MOVE_BOTTOM;
	/*int x = g_SessionNo * 100;
	int y = g_SessionNo * 100;*/
	user->x = x;
	user->y = y;
	int sectorX = x / dfSECTOR_SIZE;
	int sectorY = y / dfSECTOR_SIZE;
	user->curSector.x = sectorX;
	user->curSector.y = sectorY;
	user->oldSector.x = sectorX;
	user->oldSector.y = sectorY;
	user->moveDirection = dfAction_STAND;
	user->action = dfAction_STAND;
	session->mLastRecvTime = GetTickCount64();

	Sector_AddUser(user);

	if ((rand() & 0x1) == 1)
	{
		user->direction = dfACTION_MOVE_RR;
	}
	else
	{
		user->direction = dfACTION_MOVE_LL;
	}

	user->sessionNo = g_SessionNo;
	user->userNo = g_SessionNo;

	InsertSessionData(g_SessionNo, session);
	InsertUserData(g_SessionNo, user);

	// 캐릭터 생성 메세지
	CPacket packet;
	cpSC_CreateMyUser(&packet, user->userNo, user->direction, user->x, user->y, user->hp);
	SendPacket_Unicast(user->userNo, &packet);

	// 주변 섹터 메시지 전송
	packet.Clear();
	cpSC_CreateOtherUser(&packet, user->userNo, user->direction, user->x, user->y, user->hp);
	SendPacket_Around(user->userNo, &packet);

	st_Sector_Around sectorAround;
	GetSectorAround(sectorX, sectorY, &sectorAround);

	for (int i = 0; i < sectorAround.count; ++i)
	{
		for (auto iter = g_Sector[sectorAround.around[i].y][sectorAround.around[i].x].begin();
			iter != g_Sector[sectorAround.around[i].y][sectorAround.around[i].x].end(); iter++)
		{
			User* other = *iter;
			if (other == user)
				continue;

			packet.Clear();
			cpSC_CreateOtherUser(&packet, other->userNo, other->direction, other->x, other->y, other->hp);

			SendPacket_Unicast(user->userNo, &packet);
		}
	}

	g_SessionNo++;
}

void LogError(const WCHAR* msg, SOCKET sock, int logLevel)
{
	int err = WSAGetLastError();
	g_Logger._Log(logLevel, L"%s > %d\n", msg, err);
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
	}
}

void SendPacket_SectorOne(int sectorX, int sectorY, CPacket* packet, DWORD exceptSessionNo)
{
	for (auto iter = g_Sector[sectorY][sectorX].begin(); iter != g_Sector[sectorY][sectorX].end(); ++iter)
	{
		if ((*iter)->sessionNo == exceptSessionNo)
			continue;

		SendPacket_Unicast((*iter)->sessionNo, packet);
#ifdef DEBUG
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sector Sent Message [X: %d][Y: %d][userNo: %d]\n",
			sectorX, sectorY, (*iter)->userNo);
#endif
	}
}

void SendPacket_Unicast(DWORD to, CPacket* packet)
{
	Session* session = FindSession(to);
	User* user = FindUser(to);

	if (session == nullptr || user == nullptr)
	{
		g_Logger._Log(dfLOG_LEVEL_NOTICE, L"SendUnicast Dest Client is Null\n");
		return;
	}

	session->sendPacket(packet->GetBufferPtr(), packet->GetSize());
}

void SendPacket_Around(DWORD to, CPacket* packet, bool sendMyself)
{
	User* user = FindUser(to);
	st_Sector_Around sectors;

	GetSectorAround(user->curSector.x, user->curSector.y, &sectors);

	if (sendMyself)
	{
		for (int i = 0; i < sectors.count; ++i)
		{
			SendPacket_SectorOne(sectors.around[i].x, sectors.around[i].y, packet, 0);
		}
	}
	else
	{
		for (int i = 0; i < sectors.count; ++i)
		{
			SendPacket_SectorOne(sectors.around[i].x, sectors.around[i].y, packet, to);
		}
	}
	
}

void SendPacket_Broadcast(CPacket* packet)
{
	for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter)
	{
		if (iter->second == nullptr)
			continue;

		iter->second->sendPacket(packet->GetBufferPtr(), packet->GetSize());
	}
}

void UserSectorUpdatePacket(User* user)
{
	st_Sector_Around removeSector;
	st_Sector_Around addSector;
	CPacket packet;

	GetUpdateSectorAround(user, &removeSector, &addSector);

	// 섹터 삭제 추가 로그
#ifdef DEBUG
	for (int cnt = 0; cnt < removeSector.count; ++cnt)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Removed Sector [X: %d][Y: %d]\n",
			removeSector.around[cnt].x, removeSector.around[cnt].y);
	}
	for (int cnt = 0; cnt < addSector.count; ++cnt)
	{
		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Added Sector [X: %d][Y: %d]\n",
			addSector.around[cnt].x, addSector.around[cnt].y);
	}
#endif

	cpSC_DeleteUser(&packet, user->userNo);

	for (int cnt = 0; cnt < removeSector.count; ++cnt)
	{
		SendPacket_SectorOne(removeSector.around[cnt].x, removeSector.around[cnt].y, &packet, 0);
	}

	for (int cnt = 0; cnt < removeSector.count; ++cnt)
	{
		auto* userList = &g_Sector[removeSector.around[cnt].y][removeSector.around[cnt].x];

		for (auto iter = userList->begin(); iter != userList->end(); ++iter)
		{
			packet.Clear();
			cpSC_DeleteUser(&packet, (*iter)->userNo);
			SendPacket_Unicast(user->sessionNo, &packet);
		}
	}

	packet.Clear();
	cpSC_CreateOtherUser(&packet, user->sessionNo, user->direction, user->x, user->y, user->hp);

	for (int cnt = 0; cnt < addSector.count; ++cnt)
	{
		SendPacket_SectorOne(addSector.around[cnt].x, addSector.around[cnt].y, &packet, 0);
	}

	for (int cnt = 0; cnt < addSector.count; ++cnt)
	{
		auto* userList = &g_Sector[addSector.around[cnt].y][addSector.around[cnt].x];

		for (auto iter = userList->begin(); iter != userList->end(); ++iter)
		{
			packet.Clear();
			
			User* pUser = *iter;

			if (pUser == user)
				continue;

			cpSC_CreateOtherUser(&packet, pUser->userNo, pUser->direction, pUser->x, pUser->y, pUser->hp);
			SendPacket_Unicast(user->sessionNo, &packet);

			packet.Clear();
			switch (pUser->action)
			{
			case dfACTION_MOVE_LL:
			case dfACTION_MOVE_LU:
			case dfACTION_MOVE_UU:
			case dfACTION_MOVE_RU:
			case dfACTION_MOVE_RR:
			case dfACTION_MOVE_RD:
			case dfACTION_MOVE_DD:
			case dfACTION_MOVE_LD:
				cpSC_MoveStart(&packet, pUser->userNo, pUser->moveDirection, pUser->x, pUser->y);
				SendPacket_Unicast(user->userNo, &packet);
				break;
			}
		}
	}
}
