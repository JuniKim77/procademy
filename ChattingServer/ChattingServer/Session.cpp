#include "Session.h"
#include "Protocol.h"
#include "CPacket.h"
#include <unordered_map>
#include "User.h"
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

void Session::receivePacket()
{
	int freeSize = mRecvBuffer.GetFreeSize();
	int directEnqueueSize = mRecvBuffer.DirectEnqueueSize();

	if (freeSize > directEnqueueSize)
	{
		if (ReceiveHelper(directEnqueueSize) == false)
		{
			return;
		}
		if (ReceiveHelper(freeSize - directEnqueueSize) == false)
		{
			return;
		}
	}
	else
	{
		if (ReceiveHelper(freeSize) == false)
		{
			return;
		}
	}

	while (1)
	{
		if (receiveProc() == false)
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
		wprintf_s(L"[UserNo: %d] Enqueue 에러\n", mSessionNo);
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
		wprintf(L"Code Error [UserNo: %d]\n", mSessionNo);

		SetDisconnect();

		return false;
	}

	if (mRecvBuffer.GetUseSize() < header.wPayloadSize + sizeof(header))
		return false;

	mRecvBuffer.MoveFront(sizeof(header));

	return readMessage(&header);
}

void Session::writeProc()
{
	if (mSendCount == 5)
	{
		int test = 0;
	}
	if (mSendCount == 6)
	{
		int test = 0;
	}
	if (mSendCount == 7)
	{
		int test = 0;
	}

	if (mSendBuffer.GetUseSize() < mRecvSizes.top())
		return;

	int directPoss = mSendBuffer.DirectDequeueSize();

	int sendSize = send(mSocket, mSendBuffer.GetFrontBufferPtr(), directPoss, 0);

	mSendBuffer.MoveFront(sendSize);

	if (mSendBuffer.GetUseSize() > 0)
	{
		int sendSize2 = send(mSocket, mSendBuffer.GetFrontBufferPtr(), mSendBuffer.GetUseSize(), 0);

		mSendBuffer.MoveFront(sendSize2);

		sendSize += sendSize2;
	}

	if (sendSize != (mRecvSizes.top() + 6))
	{
		int test = 0;
	}

	mRecvSizes.pop();

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

	mSendCount++;
}

bool Session::readMessage(st_PACKET_HEADER* header)
{
	CPacket packet;
	mRecvBuffer.Dequeue(packet.GetBufferPtr(), header->wPayloadSize);
	packet.MoveRear(header->wPayloadSize);

	mCheckSums.push(header->byCheckSum);
	mRecvSizes.push(header->wPayloadSize);
	mRecvHeaders.push_back(*header);

	BYTE checkSum = makeCheckSum(&packet, header->wMsgType);
	if (checkSum != header->byCheckSum)
	{
		wprintf(L"CheckSum Error [UserNo: %d]\n", mSessionNo);
		SetDisconnect();

		return false;
	}

	if (!PacketProc(mSessionNo, header->wMsgType, &packet))
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

	return (BYTE)(checkSum % 256);
}

bool Session::ReceiveHelper(int size)
{
	if (size == 0)
		return true;

	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), size, 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return true;

		wprintf_s(L"Error: %d\n", err);
		wprintf_s(L"Disconnect [UserNo: %d]\n", mSessionNo);
		SetDisconnect();

		return false;
	}

	mRecvBuffer.MoveRear(retval);

	return true;
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
