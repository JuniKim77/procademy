#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
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

int main()
{
	setlocale(LC_ALL, "");

	WSADATA wsa;
	int retval;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		printf("Error Code: %d\n", err);
		return 1;
	}
	/// <summary>
	/// socket option -> receive timer
	/// </summary>
	/// <returns></returns>
	int interval = 4000;
	retval = setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(interval));
	if (retval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("Error Code: %d\n", err);
		closesocket(listen_socket);
		return 1;
	}

	SOCKADDR_IN serverIP;
	serverIP.sin_family = AF_INET;
	serverIP.sin_port = htons(10010);
	serverIP.sin_addr.S_un.S_addr = INADDR_ANY;

	retval = bind(listen_socket, (SOCKADDR*)&serverIP, sizeof(serverIP));
	if (retval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("Error Code: %d\n", err);
		closesocket(listen_socket);
		return 1;
	}

	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("Error Code: %d\n", err);
		closesocket(listen_socket);
		return 1;
	}

	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen;
	char buffer[1460] = { 0, };

	while (1) 
	{
		addrLen = sizeof(clientAddr);
		client_sock = accept(listen_socket, (SOCKADDR*)&clientAddr, &addrLen);
		if (client_sock == INVALID_SOCKET) {
			int err = WSAGetLastError();
			printf("Error Code: %d\n", err);
			return 1;
		}

		WCHAR ip_buffer[16] = { 0, };
		InetNtop(AF_INET, &clientAddr.sin_addr, ip_buffer, sizeof(ip_buffer));
		wprintf_s(L"[TCP] Client 접속 IP: %s\n", ip_buffer);

		st_PACKET_HEADER header;

		retval = recv(client_sock, (char*)&header, sizeof(header), 0);

		if (retval == SOCKET_ERROR) {
			int err = WSAGetLastError();
			printf("send error\n");
			printf("Error Code: %d\n", err);
			return 1;
		}

		if (header.dwPacketCode != 0x11223344) {
			wprintf_s(L"%s\n", header.szFileName);
			wprintf_s(L"%s\n", header.szName);
			printf("Packet Code Not Matched!! code: %x\n", header.dwPacketCode);
			closesocket(client_sock);

			continue;
		}

		WCHAR file_name[64];

		swprintf_s(file_name, sizeof(file_name), L"%s_%s", header.szName, header.szFileName);

		wprintf_s(L"File Name: %s\n", file_name);
		wprintf_s(L"File Size: %d\n", header.iFileSize);

		FILE* fout;
		_wfopen_s(&fout, file_name, L"wb");

		while (1)
		{
			retval = recv(client_sock, buffer, sizeof(buffer), 0);
			if (retval == SOCKET_ERROR) {
				int err = WSAGetLastError();
				printf("Error Code: %d\n", err);
				break;
			}
			else if (retval == 0) {
				break;
			}

			printf("retval : %d\n", retval);
			fwrite(buffer, retval, 1, fout);
		}

		printf("Connection Close\n");

		fclose(fout);

		closesocket(client_sock);
	}

	WSACleanup();

	return 0;
}