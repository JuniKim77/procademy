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
	int dSize = mRecvBuffer.DirectEnqueueSize();
	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), dSize, 0);

	mRecvBuffer.MoveRear(retval);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		wprintf_s(L"Error: %d\n", err);
		wprintf_s(L"Disconnect [UserNo: %d]\n", mSessionNo);
		SetDisconnect();

		return;
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

bool Session::readMessage(st_PACKET_HEADER* header)
{
	CPacket packet;
	mRecvBuffer.Dequeue(packet.GetBufferPtr(), header->wPayloadSize);
	packet.MoveRear(header->wPayloadSize);

	BYTE checkSum = makeCheckSum(&packet, header->wMsgType);
	if (checkSum != header->byCheckSum)
	{
		wprintf(L"Read Message CheckSum Error [UserNo: %d]\n", mSessionNo);
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