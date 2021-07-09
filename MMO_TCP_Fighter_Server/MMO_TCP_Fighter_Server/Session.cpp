#include "Session.h"
#include "CPacket.h"
#include <unordered_map>
#include "User.h"
#include "PacketDefine.h"
#include "Content.h"

using namespace std;

extern unordered_map<DWORD, User*> g_users;

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
		wprintf_s(L"[UserNo: %d] Enqueue ����\n", mSessionNo);
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
		wprintf(L"Code Error [UserNo: %d]\n", mSessionNo);

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
		wprintf(L"������ ���� [UserNo: %d]\n", mSessionNo);
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

		if (sendSize == 0)
		{
			break;
		}

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
