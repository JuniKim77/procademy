#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include "Session.h"
#include "stdio.h"
#include "PacketDefine.h"

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

void Session::SendPacket(char* packet, int size)
{
	int retval = sendBuffer.Enqueue(packet, size);

	if (retval < size)
		ErrorQuit(L"Send Error: 서버 연결 장애");
}

void Session::writeProc()
{


}

void Session::ReceivePacket(char* packet, int size)
{
	char packet[10000];
	int retval = recv(mSocket, packet, recvBuffer.GetFreeSize(), 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;

		ErrorQuit(L"Receive Error: 서버 연결 장애");
	}

	recvBuffer.Enqueue(packet, retval);
}

void Session::recvProc()
{
	while (1)
	{
		if (recvBuffer.GetUseSize() < sizeof(stHeader))
			break;

		stHeader header;
		recvBuffer.Peek((char*)&header, sizeof(stHeader));

		if (header.byCode != 0x89)
			ErrorQuit(L"비정상 서버");

		if (recvBuffer.GetUseSize() < header.bySize + sizeof(stHeader))
			break;

		recvBuffer.MoveFront(sizeof(stHeader));

	}
}

void Session::ErrorQuit(const WCHAR* msg)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s code : %d\n", msg, err);

	exit(1);
}

void Session::ErrorDisplay(const WCHAR* msg)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s code : %d\n", msg, err);
}