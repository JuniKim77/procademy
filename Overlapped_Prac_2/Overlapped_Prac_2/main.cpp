// Overlapped Callback

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512

// 소켓 정보 저장을 위한 구조체
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

SOCKET client_sock;

// 소켓 입출력 함수
DWORD WINAPI WorkerThread(LPVOID arg);
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags
);
// 오류 출력 함수

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) printf("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) printf("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) printf("listen()");

	// 이벤트 객체 생성
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL) return -1;

	// 스레드 생성
	DWORD ThreadId;
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread,
		(LPVOID)hEvent, 0, &ThreadId);
	if (hThread == NULL) return -1;
	CloseHandle(hThread);

	while (1) {
		// accept()
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			printf("accept()");
			continue;
		}
		int optval = 0;
		int optlen = sizeof(optval);
		if (setsockopt(client_sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
		{
			printf("setsockopt send size change\n");
			continue;
		}
		if (!SetEvent(hEvent)) break;
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 스레드 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	HANDLE hEvent = (HANDLE)arg;
	int retval;

	while (1) {
		while (1) {
			// alertable wait
			DWORD result = WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
			if (result == WAIT_OBJECT_0) break;
			if (result != WAIT_IO_COMPLETION) return -1;
		}

		// 접속한 클라이언트 정보 출력
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);
		printf("[TCP 서버] 클라이언트 접속: 포트 번호=%d\n",
			ntohs(clientaddr.sin_port));

		// 소켓 정보 구조체 할당과 초기화
		SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL) {
			printf("[오류] 메모리가 부족합니다!\n");
			return -1;
		}
		ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = 0;
		ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// 비동기 입출력 시작
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &(ptr->wsabuf), 1, &recvbytes,
			&flags, &(ptr->overlapped), CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv()");
				return -1;
			}
		}
	}

	return 0;
}

// 완료 루틴
void CALLBACK CompletionRoutine(
	DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int retval;

	// 클라이언트 정보 얻기
	SOCKETINFO* ptr = (SOCKETINFO*)lpOverlapped;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

	// 비동기 입출력 결과 확인
	if (dwError != 0 || cbTransferred == 0) {
		closesocket(ptr->sock);
		printf("[TCP 서버] 클라이언트 종료: 포트 번호=%d\n",
			ntohs(clientaddr.sin_port));
		delete ptr;
		return;
	}

	// 데이터 전송량 갱신
	if (ptr->recvbytes == 0) {
		ptr->recvbytes = cbTransferred;
		ptr->sendbytes = 0;
		// 받은 데이터 출력
		ptr->buf[ptr->recvbytes] = '\0';
		printf("[TCP/%d] %s\n", 
			ntohs(clientaddr.sin_port), ptr->buf);
	}
	else {
		ptr->sendbytes += cbTransferred;
	}

	if (ptr->recvbytes > ptr->sendbytes) {
		// 데이터 보내기
		ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
		ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

		DWORD sendbytes;
		retval = WSASend(ptr->sock, &(ptr->wsabuf), 1, &sendbytes,
			0, &(ptr->overlapped), CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			int err = WSAGetLastError();

			if (err == WSA_IO_PENDING)
			{
				printf("Send Pending\n");
			} else  {
				printf("WSASend()");
				return;
			}
		}
	}
	else {
		ptr->recvbytes = 0;

		// 데이터 받기
		ZeroMemory(&(ptr->overlapped), sizeof(ptr->overlapped));
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &(ptr->wsabuf), 1, &recvbytes,
			&flags, &(ptr->overlapped), CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			int err = WSAGetLastError();

			if (err == WSA_IO_PENDING)
			{
				printf("Recv Pending\n");
			}
			else {
				printf("WSARecv()");
				return;
			}
		}
	}
}