#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Session.h"
#include "CPacket.h"
#include <unordered_map>
#include "User.h"
#include "PacketDefine.h"
#include "Content.h"
#include "CLogger.h"

using namespace std;

extern unordered_map<DWORD, User*> g_users;
extern CLogger g_Logger;

Session::Session(SOCKET socket, u_short port, u_long ip, DWORD sessionNo)
	: mSocket(socket)
	, mPort(port)
	, mIP(ip)
	, mSessionNo(sessionNo)
{
}

Session::~Session()
{
	if (mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
	}
}

void Session::printInfo() const
{
}

void Session::receiveProc()
{
	if (mRecvBuffer.GetFreeSize() == 0)
	{
		return;
	}

	int dSize = mRecvBuffer.DirectEnqueueSize();
	WSABUF wsabufs[2];

	wsabufs[0].buf = mRecvBuffer.GetRearBufferPtr();
	wsabufs[0].len = dSize;

	wsabufs[1].buf = mRecvBuffer.GetBuffer();
	wsabufs[1].len = mRecvBuffer.GetFreeSize() - dSize;

	DWORD recvSize = 0;
	DWORD flags = 0;

	int retval = WSARecv(mSocket, wsabufs, 2, &recvSize, &flags, NULL, NULL);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		SetDisconnect();

		return;
	}

	mRecvBuffer.MoveRear(recvSize);

	mLastRecvTime = GetTickCount64();

	// 받았으면 다 처리해준다.
	while (1)
	{
		if (completeRecvPacket() == false)
		{
			break;
		}
	}
}

void Session::sendPacket(char* buffer, int size)
{
	int enqueueSize = mSendBuffer.Enqueue(buffer, size);
	//int enqueueSize = mSendBuffer.Put(buffer, size);
	if (enqueueSize != size)
	{
		g_Logger._Log(dfLOG_LEVEL_NOTICE, L"[UserNo: %d] Send Ringbuffer is full\n", mSessionNo);
	}
}

bool Session::completeRecvPacket()
{
	if (mRecvBuffer.GetUseSize() < sizeof(stHeader))
		return false;

	stHeader header;
	mRecvBuffer.Peek((char*)&header, sizeof(header));

	if (header.byCode != dfPACKET_CODE)
	{
		// 비정상 유저
		WCHAR temp[16] = { 0, };
		InetNtop(AF_INET, &mIP, temp, 16);

		g_Logger._Log(dfLOG_LEVEL_NOTICE, L"Abnormal User: [IP: %s][Port: %d] [UserNo: %d]\n",
			temp, mPort, mSessionNo);

		SetDisconnect();

		return false;
	}

	if (mRecvBuffer.GetUseSize() < header.bySize + sizeof(header))
		return false;

	mRecvBuffer.MoveFront(sizeof(header));

	CPacket packet;
	mRecvBuffer.Dequeue(packet.GetBufferPtr(), header.bySize);
	//mRecvBuffer.Get(packet.GetBufferPtr(), header.bySize);
	packet.MoveRear(header.bySize);

	if (!PacketProc(mSessionNo, header.byType, &packet))
	{
		WCHAR temp[16] = { 0, };
		InetNtop(AF_INET, &mIP, temp, 16);

		g_Logger._Log(dfLOG_LEVEL_NOTICE, L"Abnormal User: [IP: %s][Port: %d] [UserNo: %d]\n",
			temp, mPort, mSessionNo);

		SetDisconnect();

		return false;
	}

	return true;
}

void Session::writeProc()
{
	WSABUF wsabufs[2];

	int dSize = mSendBuffer.DirectDequeueSize();

	wsabufs[0].buf = mSendBuffer.GetFrontBufferPtr();
	wsabufs[0].len = dSize;

	wsabufs[1].buf = mSendBuffer.GetBuffer();
	wsabufs[1].len = mSendBuffer.GetUseSize() - dSize;

	DWORD sendSize = 0;
	DWORD flags = 0;

	int retval = WSASend(mSocket, wsabufs, 2, &sendSize, flags, NULL, NULL);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		SetDisconnect();

		return;
	}

	mSendBuffer.MoveFront(sendSize);	 
}
