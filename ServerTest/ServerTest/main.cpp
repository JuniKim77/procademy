#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#pragma pack(push, 1)
struct st_PACKET_HEADER
{
	DWORD	dwPacketCode;		// 0x11223344	우리의 패킷확인 고정값

	WCHAR	szName[32];		// 본인이름, 유니코드 NULL 문자 끝
	WCHAR	szFileName[128];	// 파일이름, 유니코드 NULL 문자 끝
	int	iFileSize;
};
#pragma pack(pop)

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
	DomainToIP(L"procademyserver.iptime.org", &Addr);

	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr = Addr;
	SockAddr.sin_port = htons(10010);

	WCHAR serverIP[16]  = { 0, };

	InetNtop(AF_INET, &SockAddr.sin_addr, serverIP, 16);

	wprintf_s(L"아이피 주소: %s\n", serverIP);

	st_PACKET_HEADER packetHeader = { 0, };

	packetHeader.dwPacketCode = 0x11223344;
	wcscpy_s(packetHeader.szFileName, L"tiger.bmp");
	wcscpy_s(packetHeader.szName, L"김호준");

	FILE* fin;

	_wfopen_s(&fin, L"tiger.bmp", L"r");

	fseek(fin, 0, SEEK_END);
	int size = ftell(fin);
	packetHeader.iFileSize = size;
	fseek(fin, 0, SEEK_SET);

	char* imageBuffer = (char*)malloc(size);

	fread(imageBuffer, size, 1, fin);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		int code = WSAGetLastError();
		printf("Socket Create Error Code : %d\n", code);
		return 0;
	}

	int ret = connect(sock, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
	if (ret == SOCKET_ERROR) {
		int code = WSAGetLastError();
		printf("Connect Error Code : %d\n", code);
		return 0;
	}

	ret = send(sock, (char*)&packetHeader, sizeof(packetHeader), 0);
	if (ret == SOCKET_ERROR) {
		int code = WSAGetLastError();
		printf("Send Error Code : %d\n", code);
		return 0;
	}

	int count = 0;
	const char* pImage = imageBuffer;
	char buffer[1460];

	printf("Image Size %d\n", size);

	while (count < size)
	{
		int sendSize = (size - count) >= 1460 ? 1460 : size - count;

		ret = send(sock, pImage, sendSize, 0);
		if (ret == SOCKET_ERROR) {
			int code = WSAGetLastError();
			printf("Send Error Code : %d\n", code);
			return 0;
		}

		pImage += sendSize;
		count += sendSize;

		printf("Send Size : %d\nTotal Count : %d\n", sendSize, count);
	}

	closesocket(sock);
	fclose(fin);

	free(imageBuffer);
	WSACleanup();

	return 0;
}