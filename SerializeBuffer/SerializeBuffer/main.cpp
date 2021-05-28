#include "CPacket.h"

int main()
{
	CPacket packet;
	unsigned char t1 = 100;
	unsigned char t1_1 = 200;

	packet << t1;

	char t2 = 400;
	char t2_1 = 500;

	packet << t2;

	short t3 = 0xAAAA;
	short t3_1 = 200;

	packet << t3;

	packet >> t1_1;
	packet >> t2_1;
	packet >> t3_1;

	
	return 0;
}