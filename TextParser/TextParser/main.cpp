#include "TextParser.h"
#include <locale.h>

int main()
{
	_wsetlocale(LC_ALL, L"");
	TextParser tp;
	int bindPort;
	int bindPort_2;
	int wTh;
	int wTh_2;
	int aTh;
	int aTh_2;
	WCHAR bindIP[MAX_PARSER_LENGTH];
	WCHAR bindIP_2[MAX_PARSER_LENGTH];

	tp.LoadFile(L"ChatServer.cnf");

	tp.GetValue(L"BIND_IP", L"SERVER_2", bindIP_2);
	tp.GetValue(L"BIND_PORT", L"SERVER_2", &bindPort_2);
	tp.GetValue(L"IOCP_ACTIVE_THREAD", L"SERVER_2", &aTh_2);
	tp.GetValue(L"IOCP_WORKER_THREAD", L"SERVER_2", &wTh_2);

	tp.GetValue(L"BIND_IP", L"SERVER", bindIP);
	tp.GetValue(L"BIND_PORT", L"SERVER", &bindPort);
	tp.GetValue(L"IOCP_ACTIVE_THREAD", L"SERVER", &aTh);
	tp.GetValue(L"IOCP_WORKER_THREAD", L"SERVER", &wTh);

	return 0;
}