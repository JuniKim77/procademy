#include "TextParser.h"
#include <locale.h>

int main()
{
	_wsetlocale(LC_ALL, L"");
	TextParser tp;
	int bindPort;
	int wTh;
	int aTh;
	int clientMax;
	int pCode;
	int pKey;
	int timeout;
	int moNum;
	WCHAR bindIP[MAX_PARSER_LENGTH];
	WCHAR log_level[MAX_PARSER_LENGTH];

	tp.LoadFile(L"ChatServer.cnf");
	tp.GetValue(L"BIND_IP", bindIP);
	tp.GetValue(L"BIND_PORT", &bindPort);
	tp.GetValue(L"LOG_LEVEL", log_level);
	tp.GetValue(L"IOCP_ACTIVE_THREAD", &aTh);
	tp.GetValue(L"CLIENT_MAX", &clientMax);
	tp.GetValue(L"IOCP_WORKER_THREAD", &wTh);
	tp.GetValue(L"MONITOR_NO", &moNum);
	tp.GetValue(L"PACKET_CODE", &pCode);
	tp.GetValue(L"PACKET_KEY", &pKey);
	tp.GetValue(L"TIMEOUT_DISCONNECT", &timeout);

	return 0;
}