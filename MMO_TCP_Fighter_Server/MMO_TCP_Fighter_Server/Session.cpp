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
	int dSize = mRecvBuffer.DirectEnqueueSize();
	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), dSize, 0);

	// Select �𵨿��� ������ �԰�, �������� ������ �ִµ�, retval�� 0�ΰŸ� ���� �޼���!
	// ���� �ƹ��͵� ó�� ���ϰ� ����.
	if (mRecvBuffer.GetRearBufferPtr() > 0 && retval == 0)
	{
		SetDisconnect();

		return;
	}
	// ���� �����ε�, ������� �ƴϸ� ���� �̻��� ��! -> ����!
	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		SetDisconnect();

		return;
	}

	mRecvBuffer.MoveRear(retval);
	mLastRecvTime = GetTickCount64();
	// �޾����� �� ó�����ִ°� �⺻
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
		// ������ ����
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
	int totalSendSize = 0;

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
		totalSendSize += sendSize;
	}

	if (totalSendSize == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		SetDisconnect();

		return;
	}
}
