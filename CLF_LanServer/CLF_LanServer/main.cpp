#include "CLanEchoServer.h"
#include "CCrashDump.h"

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	procademy::CLanEchoServer server;

	server.BeginServer();

	return 0;
}