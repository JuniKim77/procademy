#include <stdio.h>
#include "CEchoServerNoLock.h"

#define dfSERVER_PORT (6000)

int main()
{
	CEchoServerNoLock server;

	server.Start(dfSERVER_PORT, 3, 2, false, 200);

	server.WaitForThreadsFin();

	return 0;
}