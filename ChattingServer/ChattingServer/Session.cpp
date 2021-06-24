#include "Session.h"
#include "Protocol.h"
#include "CPacket.h"
#include <unordered_map>
#include "User.h"

using namespace std;

extern unordered_map<DWORD, User*> g_users;

Session::Session(SOCKET socket, u_short port, u_long ip)
	: mSocket(socket)
	, mPort(port)
	, mIP(ip)
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

void Session::receivePacket()
{
	int freeSize = mRecvBuffer.GetFreeSize();
	int directEnqueueSize = mRecvBuffer.DirectEnqueueSize();
	int remain = 0;
	int size = 0;

	if (freeSize > directEnqueueSize)
	{
		size = directEnqueueSize;
		remain = freeSize - directEnqueueSize;
	}
	else
	{
		size = freeSize;
	}

	ReceiveHelper(size);
	ReceiveHelper(remain);

	while (1)
	{
		if (receiveProc() == false)
		{
			break;
		}
	}
}

bool Session::receiveProc()
{
	if (mRecvBuffer.GetUseSize() < sizeof(st_PACKET_HEADER))
		return false;

	st_PACKET_HEADER header;
	mRecvBuffer.Peek((char*)&header, sizeof(header));

	if (header.byCode != dfPACKET_CODE)
	{
		// 비정상 유저
		SetDisconnect();

		return false;
	}

	if (mRecvBuffer.GetUseSize() < header.wPayloadSize + sizeof(header))
		return false;

	mRecvBuffer.MoveFront(sizeof(header));

	bool result = readMessage(&header);

	return true;
}

void Session::writePacket()
{
	int useSize = mSendBuffer.GetUseSize();
	int directDequeueSize = mSendBuffer.DirectDequeueSize();
	int size = 0;
	int remain = 0;

	if (useSize > directDequeueSize)
	{
		size = directDequeueSize;
		remain = useSize - size;
	}
	else
	{
		size = useSize;
	}

	WriteHelper(size);
	WriteHelper(remain);
}

void Session::sendPacket(char* buffer, int size)
{
	mSendBuffer.Enqueue(buffer, size);
}

bool Session::readMessage(st_PACKET_HEADER* header)
{
	CPacket packet;
	mRecvBuffer.Dequeue(packet.GetBufferPtr(), header->wPayloadSize);
	packet.MoveRear(header->wPayloadSize);

	BYTE checkSum = makeCheckSum(&packet, header->wMsgType);
	if (checkSum != header->byCheckSum)
	{
		wprintf(L"CheckSum Error [UserNo: %d]\n", mSessionNo);
		SetDisconnect();

		return false;
	}

	if (!g_users[mSessionNo]->PacketProc(header->wMsgType, &packet))
	{
		wprintf(L"Abnormal User [UserNo: %d]\n", mSessionNo);
		SetDisconnect();

		return false;
	}

	return true;
}

BYTE Session::makeCheckSum(CPacket* packet, WORD msgType)
{
	int size = packet->GetSize();
	BYTE* pBuf = (BYTE*)packet->GetBufferPtr();
	int checkSum = msgType;

	for (int i = 0; i < size; ++i)
	{
		checkSum += *pBuf;
		pBuf++;
	}

	return checkSum;
}

void Session::ReceiveHelper(int size)
{
	if (size == 0)
		return;

	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), size, 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		SetDisconnect();

		return;
	}

	mRecvBuffer.MoveRear(retval);
}

void Session::WriteHelper(int size)
{
	if (size == 0)
		return;

	int sendSize = send(mSocket, mSendBuffer.GetFrontBufferPtr(), size, 0);

	if (sendSize == SOCKET_ERROR || sendSize < size)
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
