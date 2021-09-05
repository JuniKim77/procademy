#include <stdio.h>
#include "CEchoServer.h"

#define dfSERVER_PORT (6000)

int main()
{
	CEchoServer server;

	server.Start(dfSERVER_PORT, 3, 2, false, 200);

	server.WaitForThreadsFin();

	return 0;
}