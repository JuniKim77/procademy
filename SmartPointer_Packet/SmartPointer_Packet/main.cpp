#include "SmartPointer_Packet.h"
#include "CPacket.h"


int main()
{
	CSmartPointer_Packet t1 = new CPacket;

	t1 << 10;


	return 0;
}