#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

int DomainToIPs(WCHAR* szDomain, IN_ADDR* pAddr, int size)
{
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
	{
		return -1;
	}

	int count;

	for (count = 0; count < size; ++count) {
		if (pAddrInfo == nullptr) {
			break;
		}

		pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;

		pAddr[count] = pSockAddr->sin_addr;

		pAddrInfo = pAddrInfo->ai_next;
	}

	FreeAddrInfo(pAddrInfo);

	return count;
}

int main()
{
	setlocale(LC_ALL, "");
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKADDR_IN SockAddr;
	IN_ADDR* Addr = (IN_ADDR*)malloc(sizeof(IN_ADDR) * 4);
	memset(&SockAddr, 0, sizeof(SockAddr));
	WCHAR name[] = L"procademyserver.iptime.org";

	int count = DomainToIPs(name, Addr, 4);

	for (int i = 0; i < count; ++i) {
		WCHAR buffer[16] = { 0, };

		InetNtop(AF_INET, &Addr[i], buffer, 16);

		wprintf_s(L"아이피 주소 (%d) : %s\n", i + 1, buffer);
	}

	free(Addr);
	WSACleanup();

	return 0;
}
