#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <conio.h>
#include <WS2tcpip.h>
#include <time.h>

#define WORKER_THREAD_SIZE (10)
#define NETWORK_PORT (9000)
#define STRING_SIZE (27)

HANDLE g_hcp;
HANDLE g_ExitEvent;
SOCKET g_sessionArray[WORKER_THREAD_SIZE];

unsigned int WINAPI workerThread(LPVOID arg);
unsigned int WINAPI monitorThread(LPVOID arg);
void errorLog(const WCHAR* s);
bool connectProc(int index);

char g_str[STRING_SIZE] = "PROCADEMY CODE EDUCATION!!";

LONG g_packetCount = 0;

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		errorLog(L"WSAStartup");
		return 1;
	}

	srand(time(NULL));

	HANDLE hThreads[WORKER_THREAD_SIZE];

	g_ExitEvent = CreateEvent(nullptr, true, false, nullptr);

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, monitorThread, nullptr, 0, nullptr);

	int seeds[WORKER_THREAD_SIZE];

	for (int i = 0; i < WORKER_THREAD_SIZE; ++i)
	{
		seeds[i] = i;
	}

	for (int i = 1; i < WORKER_THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, &seeds[i], 0, nullptr);
	}

	while (1)
	{
		WCHAR controlKey = _getwch();
		rewind(stdin);

		if (controlKey == L'Q')
		{
			wprintf_s(L"Exit\n");

			SetEvent(g_ExitEvent);

			break;
		}
	}

	DWORD waitResult = WaitForMultipleObjects(WORKER_THREAD_SIZE, hThreads, TRUE, INFINITE);

	switch (waitResult)
	{
	case WAIT_FAILED:
		wprintf_s(L"Main Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		wprintf_s(L"Main Thread Timeout Error\n");
		break;
	case WAIT_OBJECT_0:
		wprintf_s(L"None Error\n");
		break;
	default:
		break;
	}

	//------------------------------------------------
	// 디버깅용 코드  스레드 정상종료 확인.
	//------------------------------------------------
	DWORD ExitCode;

	wprintf_s(L"\n\n--- THREAD CHECK LOG -----------------------------\n\n");

	for (int i = 0; i < WORKER_THREAD_SIZE; ++i)
	{
		GetExitCodeThread(hThreads[i], &ExitCode);
		if (ExitCode != 0)
			wprintf_s(L"error - Worker Thread not exit\n");
	}

	return 0;
}

unsigned int __stdcall workerThread(LPVOID arg)
{
	int index = *(int*)arg;
	wprintf_s(L"Index: %d\n", index);


	srand(index * 1000);

	if (!connectProc(index))
	{
		return 1;
	}

	while (1)
	{
		int ranNum = rand() % 200 + 500;

		int retval = WaitForMultipleObjects(1, &g_ExitEvent, false, ranNum);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error\n");
			return -1;
		case WAIT_TIMEOUT:
		{
			ranNum = rand() % 50;

			if (ranNum == 27)
			{
				wprintf_s(L"Close Socket [%d]\n", g_sessionArray[index]);
				closesocket(g_sessionArray[index]);

				if (!connectProc(index))
				{
					return 1;
				}
			}
			else
			{
				WSABUF wsabuf;
				wsabuf.buf = g_str;
				wsabuf.len = 1 + rand() % (STRING_SIZE - 1);
				DWORD sentSize = 0;

				int retval_send = WSASend(g_sessionArray[index], &wsabuf, 1, &sentSize, 0, nullptr, nullptr);

				if (retval_send == SOCKET_ERROR)
				{
					errorLog(L"WSASend");

					return 1;
				}

				InterlockedAdd(&g_packetCount, sentSize);

				char buffer[STRING_SIZE];

				WSABUF wsabufSend;
				wsabufSend.buf = buffer;
				wsabufSend.len = STRING_SIZE;
				DWORD recvSize = 0;
				DWORD flags = 0;

				int retval_recv = WSARecv(g_sessionArray[index], &wsabufSend, 1, &recvSize, &flags, nullptr, nullptr);

				if (retval_recv == SOCKET_ERROR)
				{
					errorLog(L"WSASend");

					return 1;
				}

				InterlockedAdd(&g_packetCount, recvSize);
			}

			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		default:
			break;
		}
	}

	return 0;
}

unsigned int __stdcall monitorThread(LPVOID arg)
{
	while (1)
	{
		int retval = WaitForMultipleObjects(1, &g_ExitEvent, false, 1000);

		switch (retval)
		{
		case WAIT_FAILED:
			return 1;
		case WAIT_TIMEOUT:
		{
			int cur = g_packetCount;

			InterlockedExchange(&g_packetCount, 0);

			wprintf_s(L"==============================\n");
			wprintf_s(L"[Packet / sec : %d]\n", cur);
			wprintf_s(L"==============================\n\n");

			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		default:
			break;
		}
	}

	return 0;
}

void errorLog(const WCHAR* s)
{
	int err = WSAGetLastError();

	wprintf_s(L"%s, Error Code: %d\n", s, err);
}

bool connectProc(int index)
{
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);

	if (client == INVALID_SOCKET)
	{
		errorLog(L"Socket Create");

		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NETWORK_PORT);
	
	InetPton(AF_INET, L"127.0.0.1", &addr.sin_addr);

	int connectRetval = connect(client, (SOCKADDR*)&addr, sizeof(addr));

	if (connectRetval == SOCKET_ERROR)
	{
		errorLog(L"Connection Error");

		return false;
	}

	g_sessionArray[index] = client;

	return true;
}
