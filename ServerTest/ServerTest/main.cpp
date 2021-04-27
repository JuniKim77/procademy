#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

bool DomainToIP(const WCHAR* domainName, IN_ADDR* pAddr)
{
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;
	if (GetAddrInfo(domainName, L"0", NULL, &pAddrInfo) != 0)
	{
		return false;
	}

	pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;
	FreeAddrInfo(pAddrInfo);
	return true;
}

int main()
{
	setlocale(LC_ALL, "");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;


	SOCKADDR_IN SockAddr;
	IN_ADDR Addr;
	memset(&SockAddr, 0, sizeof(SockAddr));
	DomainToIP(L"google.com", &Addr);

	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr = Addr;
	SockAddr.sin_port = htons(80);

	int test = 0;

}