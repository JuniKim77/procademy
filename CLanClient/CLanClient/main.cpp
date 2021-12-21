#include "CLanEchoClient.h"
#include "CLanPacket.h"

int main()
{
	procademy::CLanEchoClient client;

	client.Connect(L"127.0.0.1", 6000);


	while (1)
	{
		procademy::CLanPacket* packet = procademy::CLanPacket::AllocAddRef();

		*packet << (int)1;

		packet->SetHeader();
		client.SendPacket(packet);
		Sleep(10);
		packet->SubRef();
	}
	

	return 0;
}