#include "Session.h"
#include "Protocol.h"

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
}

void Session::receiveProc()
{
	if (mRecvBuffer.GetUseSize() < sizeof(st_PACKET_HEADER))
		return;

	st_PACKET_HEADER header;
	mRecvBuffer.Peek((char*)&header, sizeof(header));

	if (header.byCode != 0x89)
	{
		// 비정상 유저
		SetDisconnect();
	}

	if (mRecvBuffer.GetUseSize() < header.wPayloadSize + sizeof(header))
		return;

	mRecvBuffer.MoveFront(sizeof(header));

	readMessage();
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
