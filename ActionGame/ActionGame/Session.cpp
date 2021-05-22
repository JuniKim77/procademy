#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include "Session.h"
#include "stdio.h"
#include "PacketDefine.h"

extern HWND gMainWindow;

Session::Session()
	: mSocket(INVALID_SOCKET)
{
}

Session::~Session()
{
}

bool Session::Connect(HWND hWnd)
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return false;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
		ErrorQuit(L"소켓 생성 에러");

	WCHAR ServerIP[16];
	wprintf_s(L"서버 IP: ");
	_getws_s(ServerIP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	InetPton(AF_INET, ServerIP, &addr.sin_addr);

	int asyncselectRetval = WSAAsyncSelect(mSocket, hWnd, WM_SOCKET,
		FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);

	if (asyncselectRetval == SOCKET_ERROR) {
		ErrorDisplay(L"소켓 모델 전환 에러");
		return false;
	}

	int connectRetval = connect(mSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (connectRetval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			ErrorDisplay(L"연결 에러");
			return false;
		}
	}

	return true;
}

void Session::Disconnect()
{
	closesocket(mSocket);
}

void Session::SendPacket(char* packet, int size)
{
	int retval = mSendBuffer.Enqueue(packet, size);

	if (retval < size)
		ErrorQuit(L"Send Error: 서버 연결 장애");
}

void Session::writeProc()
{
	char buffer[10000];
	int peekSize = mSendBuffer.Peek(buffer, 10000);

	if (peekSize == 0)
		return;

	int sendSize = send(mSocket, buffer, peekSize, 0);

	if (sendSize == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		ErrorQuit(L"Send Error");
	}

	mSendBuffer.MoveFront(sendSize);
}

void Session::ReceivePacket()
{
	char buffer[10000];
	int retval = recv(mSocket, buffer, mRecvBuffer.GetFreeSize(), 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;

		ErrorQuit(L"Receive Error: 서버 연결 장애");
	}

	mRecvBuffer.Enqueue(buffer, retval);
}

void Session::recvProc()
{
	while (1)
	{
		if (mRecvBuffer.GetUseSize() < sizeof(stHeader))
			break;

		stHeader header;
		mRecvBuffer.Peek((char*)&header, sizeof(stHeader));

		if (header.byCode != 0x89)
			ErrorQuit(L"비정상 서버");

		if (mRecvBuffer.GetUseSize() < header.bySize + sizeof(stHeader))
			break;

		mRecvBuffer.MoveFront(sizeof(stHeader));

	}
}

void Session::ErrorQuit(const WCHAR* msg)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	swprintf_s(errorMsg, 100, L"%s code : %d", msg, err);
	MessageBox(gMainWindow, errorMsg, L"접속종료", MB_OK);

	exit(1);
}

void Session::ErrorDisplay(const WCHAR* msg)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	swprintf_s(errorMsg, 100, L"%s code : %d", msg, err);
	MessageBox(gMainWindow, errorMsg, L"에러 발생", MB_OK);
}