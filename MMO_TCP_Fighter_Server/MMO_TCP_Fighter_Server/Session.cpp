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
extern char g_writeType;

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

	/*char buffer[10000];

	int retval = recv(mSocket, buffer, mRecvBuffer.GetFreeSize(), 0);

	mRecvBuffer.Enqueue(buffer, retval);*/

	int dSize = mRecvBuffer.DirectEnqueueSize();
	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), dSize, 0);

	// Select 모델에서 반응이 왔고, 링버퍼의 여유도 있는데, retval이 0인거면 종료 메세지!
	// 이후 아무것도 처리 안하고 나감.
	if (retval == 0)
	{
		SetDisconnect();

		return;
	}

	// 소켓 에러인데, 우드블락도 아니면 소켓 이상한 것! -> 종료!
	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		SetDisconnect();

		return;
	}

	mRecvBuffer.MoveRear(retval);

	//if (retval == dSize)
	//{
	//	//g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[UserNo: %d] Receive Enqueue Boundary..\n", mSessionNo);

	//	dSize = mRecvBuffer.DirectEnqueueSize();
	//	retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), dSize, 0);

	//	if (retval == SOCKET_ERROR)
	//	{
	//		int err = WSAGetLastError();

	//		if (err == WSAEWOULDBLOCK)
	//			return;

	//		SetDisconnect();

	//		return;
	//	}

	//	mRecvBuffer.MoveRear(retval);
	//}

	mLastRecvTime = GetTickCount64();
	// 받았으면 다 처리해주는게 기본
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
	while (1)
	{
		if (mSendBuffer.GetUseSize() == 0)
		{
			break;
		}

		int sendSize = send(mSocket, mSendBuffer.GetFrontBufferPtr(), mSendBuffer.DirectDequeueSize(), 0);

		if (sendSize == SOCKET_ERROR)
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
}
