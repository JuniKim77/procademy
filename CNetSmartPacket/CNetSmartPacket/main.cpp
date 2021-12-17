#include "CNetSmartPacket.h"
#include "CNetPacket.h"

using namespace procademy;

int main()
{
	CNetPacket* packet;
	{
		packet = CNetPacket::AllocAddRef();
		CNetSmartPacket t1 = packet;

		*t1 << 10;

		int test = 0;

		*t1 >> test;
	}
	
	int test = 0;

	return 0;
}