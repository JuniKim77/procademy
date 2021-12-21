#include "CLanEchoClient.h"
#include <stdio.h>
#include "CLanPacket.h"
#include <cstdint>

int count = 0;

procademy::CLanEchoClient::CLanEchoClient()
{
	Start();
}

procademy::CLanEchoClient::~CLanEchoClient()
{
}

void procademy::CLanEchoClient::OnEnterJoinServer()
{
	int test = 0;
}

void procademy::CLanEchoClient::OnLeaveServer()
{
	printf("Exit\n");
}

void procademy::CLanEchoClient::OnRecv(CLanPacket* packet)
{
	if (count++ == 0)
	{
		int64_t value;

		*packet >> value;

		if (value == 0x7fffffffffffffff)
		{
			printf("pass\n");
		}
		else
		{
			printf("fail\n");
		}
	}
	else
	{
		int test = 0;

		*packet >> test;

		printf("%d\n", test);
	}
}

void procademy::CLanEchoClient::OnSend(int sendsize)
{
}

void procademy::CLanEchoClient::OnError(int errorcode, const WCHAR*)
{
}
