// Overlapped Event

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512

// ���� ���� ������ ���� ����ü
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION cs;

// ���� ����� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);
// ���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int index);

int main(int argc, char* argv[])
{
	int retval;
	InitializeCriticalSection(&cs);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) printf("socket()\n");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) printf("bind()\n");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) printf("listen()\n");

	// ����(dummy) �̺�Ʈ ��ü ����
	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
		printf("WSACreateEvent()\n");
	EventArray[nTotalSockets++] = hEvent;

	// ������ ����
	DWORD ThreadId;
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread,
		NULL, 0, &ThreadId);
	if (hThread == NULL) return -1;
	CloseHandle(hThread);

	while (1) {
		// accept()
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			printf("accept()\n");
			continue;
		}
		printf("[TCP ����] Ŭ���̾�Ʈ ����: ��Ʈ ��ȣ=%d\n",
			ntohs(clientaddr.sin_port));

		// ���� ���� �߰�
		if (AddSocketInfo(client_sock) == FALSE) {
			closesocket(client_sock);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: ��Ʈ ��ȣ=%d\n",
				ntohs(clientaddr.sin_port));
			continue;
		}

		// �񵿱� ����� ����
		SOCKETINFO* ptr = SocketInfoArray[nTotalSockets - 1];
		DWORD recvbytes;
		DWORD flags = 0;
		int retval = WSARecv(ptr->sock, &(ptr->wsabuf), 1, &recvbytes,
			&flags, &(ptr->overlapped), NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv()\n");
				RemoveSocketInfo(nTotalSockets - 1);
				continue;
			}
		}

		// ������ ����(nTotalSockets) ��ȭ�� �˸�
		if (WSASetEvent(EventArray[0]) == FALSE) {
			printf("WSASetEvent()\n");
			break;
		}
	}

	// ���� ����
	WSACleanup();
	DeleteCriticalSection(&cs);
	return 0;
}

// �񵿱� ����� ó��
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1) {
		// �̺�Ʈ ��ü ����
		DWORD index = WSAWaitForMultipleEvents(nTotalSockets,
			EventArray, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents()\n");
			continue;
		}
		index -= WSA_WAIT_EVENT_0;
		WSAResetEvent(EventArray[index]);
		if (index == 0) continue;

		// Ŭ���̾�Ʈ ���� ���
		SOCKETINFO* ptr = SocketInfoArray[index];
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

		// �񵿱� ����� ��� Ȯ��
		DWORD cbTransferred, flags;
		retval = WSAGetOverlappedResult(ptr->sock, &(ptr->overlapped),
			&cbTransferred, FALSE, &flags);
		if (retval == FALSE || cbTransferred == 0) {
			if (retval == FALSE)
				printf("WSAGetOverlappedResult()");
			RemoveSocketInfo(index);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: ��Ʈ ��ȣ=%d\n",
				ntohs(clientaddr.sin_port));
			continue;
		}

		// ������ ���۷� ����
		if (ptr->recvbytes == 0) {
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			// ���� ������ ���
			ptr->buf[ptr->recvbytes] = '\0';
			printf("[TCP:%d] %s\n",
				ntohs(clientaddr.sin_port), ptr->buf);
		}
		else {
			ptr->sendbytes += cbTransferred;
		}

		if (ptr->recvbytes > ptr->sendbytes) {
			// ������ ������
			ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
			ptr->overlapped.hEvent = EventArray[index];
			ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
			ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

			DWORD sendbytes;
			retval = WSASend(ptr->sock, &(ptr->wsabuf), 1, &sendbytes,
				0, &(ptr->overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSASend()");
				}
				continue;
			}
		}
		else {
			ptr->recvbytes = 0;

			// ������ �ޱ�
			ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
			ptr->overlapped.hEvent = EventArray[index];
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;

			DWORD recvbytes;
			flags = 0;
			retval = WSARecv(ptr->sock, &(ptr->wsabuf), 1, &recvbytes,
				&flags, &(ptr->overlapped), NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSARecv()");
				}
				continue;
			}
		}
	}
}

// ���� ���� �߰�
BOOL AddSocketInfo(SOCKET sock)
{
	EnterCriticalSection(&cs);

	if (nTotalSockets >= WSA_MAXIMUM_WAIT_EVENTS) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		LeaveCriticalSection(&cs);
		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
		LeaveCriticalSection(&cs);
		return FALSE;
	}

	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT) {
		printf("WSACreateEvent()");
		LeaveCriticalSection(&cs);
		return FALSE;
	}

	ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
	ptr->overlapped.hEvent = hEvent;
	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	ptr->wsabuf.buf = ptr->buf;
	ptr->wsabuf.len = BUFSIZE;
	SocketInfoArray[nTotalSockets] = ptr;
	EventArray[nTotalSockets] = hEvent;
	nTotalSockets++;

	LeaveCriticalSection(&cs);
	return TRUE;
}

// ���� ���� ����
void RemoveSocketInfo(int index)
{
	EnterCriticalSection(&cs);

	SOCKETINFO* ptr = SocketInfoArray[index];
	closesocket(ptr->sock);
	delete ptr;
	WSACloseEvent(EventArray[index]);

	for (int i = index; i < nTotalSockets; i++) {
		SocketInfoArray[i] = SocketInfoArray[i + 1];
		EventArray[i] = EventArray[i + 1];
	}
	nTotalSockets--;

	LeaveCriticalSection(&cs);
}